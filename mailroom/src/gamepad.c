#include "../include/log.h"
#include "../include/core.h"
#include "../include/gamepad.h"
#include <fcntl.h>
#include <errno.h>
#include <finite/input/gamepad.h>
#include <linux/input-event-codes.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
#include <libudev.h>
#include <libevdev/libevdev.h>
#include <finite/input.h>


static FiniteIPCServer server = {0};

/*
    The gamepad socket has one in/out channel (FINITE_GIPC_MAIN)

    Mailman owns all the sockets (not root). Only one client may have socket_focus at one time.
*/
int initializeGamepadSocket() {
    int fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    if (!getenv("FINITE_GIPC_MAIN_DIR")) {
        // test environment: set it locally
        setenv("FINITE_GIPC_MAIN_DIR", "/run/user/1000/gp.sock", 1);
    }

    const char *path = getenv("FINITE_GIPC_MAIN_DIR");

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    // if we had an old socket destroy it
    unlink(path);

    if (bind(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) < 0) {
        perror("bind");
        close(fd);
        FINITE_LOG_ERROR("Unable to bind to socket");
        return -1;
    }


    chmod(path, 0666);

    if (listen(fd, 8) < 0) {
        perror("listen");
        close(fd);
        FINITE_LOG_ERROR("Unable to listen to socket");
        return -1;
    }

    server.server_fd = fd;
    server.owner = 0; // self
    FINITE_LOG("Socket at %s was made.", path);
    return fd;
}

static void send_signal(int fd, FiniteGIPCResponse *res) {
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;

    char *device = getenv("FINITE_GIPC_MAIN_DIR");
    if (!device) {
        FINITE_LOG_FATAL("Attempting to send message to undefined socket");
    }
    
    struct stat dev;
    if (stat(device, &dev) != 0) {
        FINITE_LOG_ERROR("Something went wrong while attempting push a message");
        perror("stat");
    }

    if (!S_ISSOCK(dev.st_mode)) {
        FINITE_LOG_ERROR("Unable to connect to non-socket device %s", device);
    }

    if (!device) {
        FINITE_LOG_WARN("Unable to open the FINITE_GIPC_MAIN_DIR socket.");
        close(fd);
        return;
    }

    strcpy(addr.sun_path, device);
    
    if (send(fd, res, sizeof(FiniteGIPCResponse), 0) != sizeof(FiniteGIPCResponse)) {
        perror("send");
    }
}

const static FiniteGamepadKeyMapping finite_gamepad_key_lookup_table[] = {
    // letter buttons
    {"ButtonA", FINITE_BTN_A, BTN_A},
    {"ButtonB", FINITE_BTN_B, BTN_B},
    {"ButtonX", FINITE_BTN_X, BTN_X},
    {"ButtonY", FINITE_BTN_Y, BTN_Y},
    
    // DPad Input
    {"ButtonUp", FINITE_BTN_UP, BTN_DPAD_UP},
    {"ButtonDown", FINITE_BTN_DOWN, BTN_DPAD_DOWN},
    {"ButtonLeft", FINITE_BTN_LEFT, BTN_DPAD_LEFT},
    {"ButtonRight", FINITE_BTN_RIGHT, BTN_DPAD_RIGHT},

    // Triggers
    {"ButtonShoulderRight", FINITE_BTN_RIGHT_SHOULDER, BTN_TR},
    {"ButtonShoulderLeft", FINITE_BTN_LEFT_SHOULDER, BTN_TL},
    {"ButtonTriggerRight", FINITE_BTN_RIGHT_TRIGGER, BTN_TR2},
    {"ButtonTriggerLeft", FINITE_BTN_LEFT_TRIGGER, BTN_TL2},
    // {"ButtonSpecialRight", FINITE_BTN_RIGHT_SPECIAL, BTN_TL},
    // {"ButtonSpecialLeft", FINITE_BTN_LEFT_SPECIAL, BTN_TL},

    // Joystick
    {"ButtonJoystickRight", FINITE_BTN_RIGHT_JOYSTICK, BTN_THUMBR},
    {"ButtonJoystickLeft", FINITE_BTN_LEFT_JOYSTICK, BTN_THUMBL},

    // Meta
    {"ButtonStart", FINITE_BTN_START, BTN_START},
    {"ButtonSelect", FINITE_BTN_SELECT, BTN_SELECT},
    {"ButtonHome", FINITE_BTN_HOME, BTN_MODE},
};

static uint16_t finite_gamepad_key_to_evdev(FiniteGamepadKey key) {
    size_t count = sizeof(finite_gamepad_key_lookup_table) / sizeof(finite_gamepad_key_lookup_table[0]);
    for (size_t i = 0; i < count; i++) {
        if (finite_gamepad_key_lookup_table[i].key == key) {
            return finite_gamepad_key_lookup_table[i].evdev_code;
        }
    }
    return UINT16_MAX; // Not found
}

static const char *finite_gamepad_evdev_to_string(uint16_t code) {
    size_t count = sizeof(finite_gamepad_key_lookup_table) / sizeof(finite_gamepad_key_lookup_table[0]);
    for (size_t i = 0; i < count; i++) {
        if (finite_gamepad_key_lookup_table[i].evdev_code == code) {
            return finite_gamepad_key_lookup_table[i].name;
        }
    }
    return ""; // Not found
}

FiniteGamepadInfo *get_gamepads() {
    FiniteGamepadInfo *info = calloc(1, sizeof(FiniteGamepadInfo));
    info->_gamepads = 0;

    struct udev *ud = udev_new();
    if (!ud) {
        FINITE_LOG_ERROR("Unable to create udev");
        return NULL;
    }

    struct udev_enumerate *enumerate = udev_enumerate_new(ud);
    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_scan_devices(enumerate);

    struct udev_list_entry *devs = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *entry;

    udev_list_entry_foreach(entry, devs) {
        const char *path = udev_list_entry_get_name(entry);
        struct udev_device *device = udev_device_new_from_syspath(ud, path);

        const char *devnode = udev_device_get_devnode(device);
        const char *is_joystick = udev_device_get_property_value(device, "ID_INPUT_JOYSTICK");
        // const char *subsystem = udev_device_get_subsystem(device);

        if (devnode && strncmp(devnode, "/dev/input/event", 16) == 0 && is_joystick && atoi(is_joystick) == 1) {
            FINITE_LOG_INFO("Found joystick: %s", devnode);

            FiniteGamepad *current = calloc(1, sizeof(FiniteGamepad));
            if (!current) {
                FINITE_LOG_ERROR("Unable to allocate memory for new gamepad");
                return NULL;
            }

            int fd = open(devnode, O_RDONLY | O_NONBLOCK);
            if  (fd < 0) {
                perror("open");
                FINITE_LOG_ERROR("Found gamepad %s but unable to open it.", devnode);
                return NULL;
            }

            

            current->fd = fd;
            strncpy(current->path, path, PATH_MAX);
            current->order = info->_gamepads;
            current->canInput = true;
            FiniteTrigger lt = {0};
            FiniteTrigger rt = {0};

            struct libevdev *evdev = libevdev_new();
            int rc = libevdev_set_fd(evdev, fd);
            if (rc < 0) {
                FINITE_LOG_ERROR("Unable to use libevdev (%s)",strerror(-rc));
                return NULL;
            }

            if (libevdev_has_event_code(evdev, EV_ABS, ABS_X) && libevdev_has_event_code(evdev, EV_ABS, ABS_Y) ) {
                FiniteJoystick lx = {0};
                
                const struct input_absinfo *labsx = libevdev_get_abs_info(evdev, ABS_X);
                const struct input_absinfo *labsy = libevdev_get_abs_info(evdev, ABS_Y);
                
                lx.xAxis = ABS_X;
                lx.yAxis = ABS_Y;
                lx.xMax = labsx->maximum;
                lx.xMin = labsx->minimum;
                lx.xFlat = labsx->flat;
                lx.xValue = 0.0;
                lx.yMax = labsy->maximum;
                lx.yMin = labsy->minimum;
                lx.yFlat = labsy->flat;
                lx.yValue = 0.0;
                strncpy(lx.name, "Left Joystick", 16);
                current->lAxis = lx;
            }

            if (libevdev_has_event_code(evdev, EV_ABS, ABS_RX) && libevdev_has_event_code(evdev, EV_ABS, ABS_RY) ) {
                FiniteJoystick rx = {0};
                const struct input_absinfo *rabsx = libevdev_get_abs_info(evdev, ABS_RX);
                const struct input_absinfo *rabsy = libevdev_get_abs_info(evdev, ABS_RY);
                
                rx.xAxis = ABS_RX;
                rx.yAxis = ABS_RY;
                rx.xMax = rabsx->maximum;
                rx.xMin = rabsx->minimum;
                rx.xFlat = rabsx->flat;
                rx.xValue = 0.0;
                rx.yMax = rabsy->maximum;
                rx.yMin = rabsy->minimum;
                rx.yFlat = rabsy->flat;
                rx.yValue = 0.0;
                strncpy(rx.name, "Right Joystick", 16);
                current->rAxis = rx;
            }

            // triggers are weird and can be any of the three
            if (libevdev_has_event_code(evdev, EV_ABS, ABS_Z) && libevdev_has_event_code(evdev, EV_ABS, ABS_RZ) ) {
                lt.axis = ABS_Z;
                lt.value = 0.0;
                rt.axis = ABS_RZ;
                rt.value = 0.0;
                current->lt = lt;
                current->rt = rt;
            } else if (libevdev_has_event_code(evdev, EV_ABS, ABS_HAT1X) && libevdev_has_event_code(evdev, EV_ABS, ABS_HAT1Y)) {
                lt.axis = ABS_HAT1X;
                lt.value = 0.0;
                rt.axis = ABS_HAT1Y;
                rt.value = 0.0;
                current->lt = lt;
                current->rt = rt;
            } else if (libevdev_has_event_code(evdev, EV_ABS, ABS_HAT2X) && libevdev_has_event_code(evdev, EV_ABS, ABS_HAT2Y)) {
                lt.axis = ABS_HAT2X;
                lt.value = 0.0;
                rt.axis = ABS_HAT2Y;
                rt.value = 0.0;
                current->lt = lt;
                current->rt = rt;
            }

            // dpad is usually hat0
            if (libevdev_has_event_code(evdev, EV_ABS, ABS_HAT0X) && libevdev_has_event_code(evdev, EV_ABS, ABS_HAT0Y) ) {
                FiniteDpad dpad = {0};
            
                dpad.xAxis = ABS_HAT0X;
                dpad.xValue = 0.0;
                dpad.yAxis = ABS_HAT0X;
                dpad.yValue = 0.0;

                current->dpad = dpad;
            }

            FINITE_LOG("Device Id: %d", info->_gamepads);
            info->gamepads[info->_gamepads] = current;
            info->_gamepads += 1;

            FINITE_LOG("Found devices: %d", info->_gamepads);
        }
        udev_device_unref(device);
    }

    udev_enumerate_unref(enumerate);
    udev_unref(ud);

    return info;
}

FiniteGamepadInfo *get_updates() {
    // TODO
    // in mailman use inotify to get event details
    return server.info;
}


FiniteGamepadInfo *poll_gamepads() {
    FiniteGamepadInfo *info = get_updates();

    struct input_event ev;
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);

    bool isEv = false;

    for (int i = 0; i < info->_gamepads; i++) {
        FiniteGamepad *current = info->gamepads[i];
        ssize_t d;

        while (true) {
            d = read(current->fd, &ev,  sizeof(struct input_event));
            FINITE_LOG("Reading item %d. (%ld)", i, d);
            if (d == sizeof(ev)) {
                if (ev.type == EV_KEY) {
                    FINITE_LOG_INFO("Key changed: %s", finite_gamepad_evdev_to_string(ev.code));
                    current->btns[ev.code].isDown = (ev.value == 1) ? true : false;
                    current->btns[ev.code].isUp = (ev.value == 0) ? true : false;
                    if (current->btns[ev.code].isDown == true) {
                    current->btns[ev.code].heldSTime = t.tv_sec;
                    } else {
                        // when held ended reset held time
                        current->btns[ev.code].heldSTime = 0;
                    }
                } else if (ev.type == EV_ABS) {
                    if (ev.code == current->dpad.xAxis) {
                        current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_LEFT)].isDown = ev.value > 0 ? true : false;
                        current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_LEFT)].isUp = ev.value < 0 ? true : false;
                        current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_RIGHT)].isDown = ev.value < 0 ? true : false;
                        current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_RIGHT)].isUp = ev.value > 0 ? true : false;
                        current->dpad.xValue = ev.value;
                    }

                    if (ev.code == current->dpad.yAxis) {
                        current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_DOWN)].isDown = ev.value > 0 ? true : false;
                        current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_DOWN)].isUp = ev.value < 0 ? true : false;
                        current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_UP)].isDown = ev.value < 0 ? true : false;
                        current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_UP)].isUp = ev.value > 0 ? true : false;
                        current->dpad.yValue = ev.value;
                    }

                    if (ev.code == current->lt.axis) {
                        current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_LEFT_TRIGGER)].isDown = ev.value > 0 ? true : false;
                        current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_LEFT_TRIGGER)].isUp = ev.value <= 0 ? true : false;
                    }

                    if (ev.code == current->rt.axis) {
                        current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_RIGHT_TRIGGER)].isDown = ev.value > 0 ? true : false;
                        current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_RIGHT_TRIGGER)].isUp = ev.value <= 0 ? true : false;
                    }

                    // joysticks require some math
                    if (ev.code == current->lAxis.xAxis) {
                        double norm;

                        int center = (current->lAxis.xMax + current->lAxis.xMin) / 2;
                        if (current->lAxis.xFlat > 0 && abs(ev.value - center) < current->lAxis.xFlat) {
                            norm = 0.0;
                        } else {
                            norm = (2.0 * (ev.value - current->lAxis.xMin) / (current->lAxis.xMax - current->lAxis.xMin)) - 1.0;
                        }
                        current->lAxis.xValue = norm;
                    }

                    if (ev.code == current->lAxis.yAxis) {
                        double norm;

                        int center = (current->lAxis.yMax + current->lAxis.yMin) / 2;
                        if (current->lAxis.yFlat > 0 && abs(ev.value - center) < current->lAxis.yFlat) {
                            norm = 0.0;
                        } else {
                            norm = (2.0 * (ev.value - current->lAxis.yMin) / (current->lAxis.yMax - current->lAxis.yMin)) - 1.0;
                        }
                        current->lAxis.yValue = norm;
                    }

                    if (ev.code == current->rAxis.xAxis) {
                        double norm;

                        int center = (current->rAxis.xMax + current->rAxis.xMin) / 2;
                        if (current->rAxis.xFlat > 0 && abs(ev.value - center) < current->rAxis.xFlat) {
                            norm = 0.0;
                        } else {
                            norm = (2.0 * (ev.value - current->rAxis.xMin) / (current->rAxis.xMax - current->rAxis.xMin)) - 1.0;
                        }
                        current->rAxis.xValue = norm;
                    }

                    if (ev.code == current->rAxis.yAxis) {
                        double norm;

                        int center = (current->rAxis.yMax + current->rAxis.yMin) / 2;
                        if (current->rAxis.yFlat > 0 && abs(ev.value - center) < current->rAxis.yFlat) {
                            norm = 0.0;
                        } else {
                            norm = (2.0 * (ev.value - current->rAxis.yMin) / (current->rAxis.yMax - current->rAxis.yMin)) - 1.0;
                        }
                        current->rAxis.yValue = norm;
                    }
                } else if (ev.type == EV_SYN) {
                    FINITE_LOG("Ev sent.");
                    isEv = true;
                }
            } else if (d == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                }
                perror("read");
                break;
            } else {
                break;
            }
        }
    }

    FINITE_LOG("All events handled.");

    if (isEv) {
        FiniteGIPCResponse res = {
            .msg = SERVER_OK,
            ._gamepad = info->_gamepads
        };
        
        for (int i = 0; i < info->_gamepads; i++) {
            memcpy(&res.gamepads[i], info->gamepads[i], sizeof(FiniteGamepad));
        }
        send_signal(server.client_fd, &res);
        isEv = false;
    }

    return info;
}

void *handle_input() {
    int client;
    while ((client = accept(server.server_fd, NULL, NULL)) >= 0) {
        char buf[64];

        // n is the size of the buffer
        int n;
        while((n = read(client, buf, sizeof(buf))) > 0) {

            buf[n] = 0; // add \0 

            // buf may have trailing chars. remove them
            while (n > 0 && (buf[n-1] == '\n' || buf[n-1] == '\r' || buf[n-1] == '\t')) {
                buf[n-1] = 0;
                n--;
            }

            // TODO: run additional security checks

            if (strcmp(buf, "CLIENT_REQUEST_FOCUS") == 0) {
                FINITE_LOG("Focus requested.");
                if (server.client_fd == 0 ) { 
                    if (server.client_fd == client) {
                        FiniteGIPCResponse res = {
                            .msg = SERVER_ALREADY_GRANTED_FOCUS
                        };
                        send_signal(client, &res);
                        FINITE_LOG("Replying with signal (duplicate)");

                    } else {
                        FiniteGamepadInfo *info = get_gamepads();
                        if (info == NULL) {
                            FiniteGIPCResponse res = {
                                .msg = SERVER_ERROR,
                                ._gamepad = 0
                            };

                            send_signal(client, &res);
                        } else {
                            server.client_fd = client;
                            server.info = info;
                            FINITE_LOG("Devices known: %d", info->_gamepads);
                            FiniteGIPCResponse res = {
                                .msg = SERVER_OK,
                                ._gamepad = info->_gamepads
                            };
                            memset(res.gamepads, 0, sizeof(res.gamepads));
                            memcpy(res.gamepads, info->gamepads, sizeof(FiniteGamepad) * info->_gamepads);
                            FINITE_LOG("Devices known: %d", res._gamepad);
                            FINITE_LOG("Sizeof buffer: %ld", sizeof(res));
                            FINITE_LOG("Replying with signal");
                            send_signal(client, &res);
                        }
                    }
                } else {
                    FiniteGIPCResponse res = {
                        .msg = SERVER_REQUEST_DECLINED_FOCUS
                    };
                    FINITE_LOG("Replying with signal (denying request)");
                    send_signal(client, &res);
                }
            }
            if (strcmp(buf, "CLIENT_REQUEST_POLL") == 0) {
                if (server.client_fd == client) {
                    FINITE_LOG("Polling..");

                    FiniteGamepadInfo *info = poll_gamepads();
                    FiniteGIPCResponse res = {
                        .msg = SERVER_OK,
                        ._gamepad = info->_gamepads
                    };
                    memset(res.gamepads, 0, sizeof(FiniteGamepad) * MAX_GAMEPADS);
                    for (int i = 0; i < info->_gamepads; i++) {
                        memcpy(&res.gamepads[i], info->gamepads[i], sizeof(FiniteGamepad));
                    }

                    FINITE_LOG("Path: %s", info->gamepads[0]->path);
                    FINITE_LOG("Path: %s", res.gamepads[0].path);
                    send_signal(client, &res);
                } else {
                    FiniteGIPCResponse res = {
                        .msg = SERVER_REQUEST_DECLINED_POLL
                    };
                    send_signal(client, &res);
                }
            }
        }

        FINITE_LOG("Recieved client disconnected (%d)", client);
        if (client == server.client_fd) {
            server.client_fd = 0;
            server.owner = 0;
        }
    }    
    
    return NULL;
}
