#include "input/gamepad.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <bits/pthreadtypes.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <finite/draw/cairo.h>
#include <finite/log.h>
#include <float.h>
#include <limits.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <linux/limits.h>
#include <sys/inotify.h>
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <wayland-client-protocol.h>
#include <xkbcommon/xkbcommon.h>
#include "draw/cairo.h"
#include "include/draw/window.h"
#include "include/input.h"
#include "include/draw.h"
#include "log.h"

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x) - 1) / BITS_PER_LONG) + 1)
#define TEST_BIT(bit, array) ((array[(bit) / BITS_PER_LONG] >> ((bit) % BITS_PER_LONG)) & 1)

bool canScan = true; // for hotplugging thread
bool updateRequired = false;

const FiniteGamepadKeyMapping finite_gamepad_key_lookup[] = {
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

typedef struct {
    char path[PATH_MAX];
    char *name; // eventXX\n
} FiniteDevice;

typedef struct {
    int _devices;
    FiniteDevice **devices;
} FiniteDevices;

static void *finite_gamepad_poll_buttons_async(void *data);
static void *finite_hotplug_devices_async(void *data);
static void *finite_draw_controller_popup(void *data);

static void finite_free_devices(FiniteDevices *dev) {
    for (int i = 0; i < dev->_devices; i++) {
        free(dev->devices[i]->name);
        free(dev->devices[i]);
    }
    free(dev->devices);
    free(dev);
}

static void finite_free_gamepad(FiniteGamepad *gamepad) {
    free(gamepad->lAxis);
    free(gamepad->rAxis);
    free(gamepad->dpad);
    free(gamepad->lt);
    free(gamepad->rt);
    close(gamepad->fd);
    free(gamepad);
}

static FiniteDevices *finite_enumerate_devices(const char *file, const char *func, int line) {
    DIR *dir = opendir("/dev/input");
    if (!dir) {
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func,"Unable to open device folder");
        return NULL;
    }
    struct dirent *ent;

    FiniteDevice **devnodes = NULL; // array of all connected devices
    int _devices = 0;

    while ((ent = readdir(dir)) != NULL) {
        if (strncmp(ent->d_name, "event", 5) != 0) {
            continue;
        }

        char path[PATH_MAX];
        snprintf(path, sizeof(path), "/dev/input/%s", ent->d_name);

        int device = open(path, O_RDONLY);
        if (device < 0) {
            finite_log_internal(LOG_LEVEL_ERROR, file, line, func,"Unable to get device");
            perror("open");
            continue;
        }

        unsigned long evbit[NBITS(KEY_MAX)];
        memset(evbit, 0, sizeof(evbit));

        ioctl(device, EVIOCGBIT(EV_KEY, KEY_MAX), evbit);
        
        // according to the spec only gamepads are allowed to return BTN_GAMEPAD so this is safe
        if (TEST_BIT(BTN_GAMEPAD, evbit)) {
            FiniteDevice *cur = calloc(1, sizeof(FiniteDevice));
            if (!cur) {
                finite_log_internal(LOG_LEVEL_ERROR, file, line, func,"Unable to allocate memory for new device");
                continue;
            }
            cur->name = strdup(ent->d_name);
            strcpy(cur->path, path);
            FINITE_LOG("Device %s is usable.", ent->d_name);

            FiniteDevice **tmp = realloc(devnodes, (sizeof(FiniteDevice *)) * (_devices + 1));
            FINITE_LOG("FiniteDevice for %s was made.", ent->d_name);
            if (!tmp) {
                finite_log_internal(LOG_LEVEL_ERROR, file, line, func,"Unable to allocate memory for new device array");
                continue;
            } else {
                devnodes = tmp;
            }

            devnodes[_devices] = cur;
            _devices += 1;
        }

        close(device);



    }

    closedir(dir);

    if (_devices > 0) {
        FiniteDevices *devs = calloc(1, sizeof(FiniteDevices));
        if (!devs) {
            finite_log_internal(LOG_LEVEL_ERROR, file, line, func,"Unable to allocate memory new device list");
            return NULL;
        }

        devs->_devices = _devices;
        devs->devices = devnodes;
        return devs;
    } else {
        finite_log_internal(LOG_LEVEL_WARN, file, line, func,"No controllers available");
        return NULL;
    }
}

bool finite_gamepad_init_debug(const char *file, const char *func, int line, FiniteShell *shell) {
    FiniteDevices *devices = finite_enumerate_devices(file, func, line);
    if (!devices) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "No usable devices available.");
        shell->_gamepads = 0;
        shell->gamepads = NULL;
        shell->gamepadAvailable = false;

        // ensure we can still look for new events
        pthread_t hotplug;
        pthread_t poll;
        pthread_t popup;


        pthread_create(&hotplug, NULL, finite_hotplug_devices_async, shell);
        pthread_create(&poll, NULL, finite_gamepad_poll_buttons_async, shell);

        // ask for the device
        pthread_create(&popup, NULL, finite_draw_controller_popup, shell);
        pthread_detach(popup);

        return true; // we're okay with having no gamepads
    }

    FiniteGamepad **gamepads = NULL;
    int _gamepads = 0;

    for (int ioi = 0; ioi < devices->_devices; ioi++) {
        bool exists = false;
        FiniteDevice *devnode = devices->devices[ioi]; 
        if (!devnode) {
            finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to read allocated memory from device list");
            continue;
        }
        if (_gamepads > 0 && gamepads != NULL) {
            for (int i = 0; i < _gamepads; i++) {
                if ( strcmp(gamepads[i]->path, devnode->path) == 0) {
                    // we already have this device, move on
                    exists = true;
                    break;
                }
            }
        }

        if (exists) {
            continue;
        }

        gamepads = realloc(gamepads, (sizeof(FiniteGamepad *) * (_gamepads + 1)));
        if (!gamepads) {
            finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to allocate memory for new gamepad array");
            return false;
        }

        FiniteGamepad *current = calloc(1, sizeof(FiniteGamepad));
        if (!current) {
            finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to allocate memory for new gamepad");
            return false;
        }

        int fd = open(devnode->path, O_RDONLY | O_NONBLOCK);
        if  (!fd) {
            perror("open");
            finite_log_internal(LOG_LEVEL_ERROR, file, line, func,"Found gamepad %s but unable to open it.", devnode->name);
        } else {
            finite_log_internal(LOG_LEVEL_DEBUG, file, line, func,"Found gamepad %s.", devnode->name);

        }

        current->fd = fd;
        current->path = strdup(devnode->path);
        current->order = shell->_gamepads; // first controller should be 0 (cuz its item 0 in the array)
        current->canInput = true; // by default all gamepads can send input
        current->lAxis = calloc(1, sizeof(FiniteJoystick));
        current->rAxis = calloc(1, sizeof(FiniteJoystick));
        current->lAxis->xAxis = ABS_X;
        current->lAxis->yAxis = ABS_Y;
        current->rAxis->xAxis = ABS_RX;
        current->rAxis->yAxis = ABS_RY;

        struct input_absinfo labsx;
        struct input_absinfo labsy;
        struct input_absinfo rabsx;
        struct input_absinfo rabsy;

        if (ioctl(fd, EVIOCGABS(current->lAxis->xAxis), &labsx) == 0) { 
            current->lAxis->xMax = labsx.maximum;
            current->lAxis->xMin = labsx.minimum;
            current->lAxis->xFlat = labsx.flat; 
        }
        if (ioctl(fd, EVIOCGABS(current->lAxis->yAxis), &labsy) == 0) {
            current->lAxis->yMax = labsy.maximum;
            current->lAxis->yMin = labsy.minimum;
            current->lAxis->yFlat = labsy.flat;
        }
        if (ioctl(fd, EVIOCGABS(current->rAxis->xAxis), &rabsx) == 0) {
            current->rAxis->xMax = rabsx.maximum;
            current->rAxis->xMin = rabsx.minimum;
            current->rAxis->xFlat = rabsx.flat;
        }
        if (ioctl(fd, EVIOCGABS(current->rAxis->yAxis), &rabsy) == 0) {
            current->rAxis->yMax = rabsy.maximum;
            current->rAxis->yMin = rabsy.minimum;
            current->rAxis->yFlat = rabsy.flat;
        }

        current->dpad = calloc(1, sizeof(FiniteDpad));
        current->dpad->xAxis = ABS_HAT0X;
        current->dpad->yAxis = ABS_HAT0Y;

        current->lt = calloc(1, sizeof(FiniteTrigger));
        current->lt->axis = ABS_HAT2Y;
        current->lt->value = 0;
        current->rt = calloc(1, sizeof(FiniteTrigger));
        current->rt->axis = ABS_HAT2X;
        current->rt->value = 0;

        current->shell = shell;


        gamepads[_gamepads] = current;
        shell->gamepads = gamepads;
        if (shell->_gamepads == 0) {
            shell->primaryGamepadId = 0;
        }
        _gamepads += 1;
        shell->_gamepads = _gamepads;




    }

    finite_free_devices(devices);

    // whether or not we have gamepads doesn't matter right now, attach the threads
    if (_gamepads > 0) {
        shell->gamepadAvailable = true;
    } else {
        shell->gamepadAvailable = false;
    } 

    shell->canInput = true;

    pthread_t hotplug;
    pthread_t poll;

    pthread_create(&hotplug, NULL, finite_hotplug_devices_async, shell);
    pthread_create(&poll, NULL, finite_gamepad_poll_buttons_async, shell);

    return true;
}

static bool finite_gamepad_reinit(FiniteShell *shell) {
    FiniteDevices *devices = finite_enumerate_devices(__FILE__,__func__, __LINE__);
    if (!devices) {
        FINITE_LOG_ERROR("No usable devices available.");
        shell->_gamepads = 0;
        shell->gamepads = NULL;
        shell->gamepadAvailable = false;
        return true; // we're okay with having no gamepads
    }

    FiniteGamepad **gamepads = NULL;
    int _gamepads = 0;

    for (int ioi = 0; ioi < devices->_devices; ioi++) {
        bool exists = false;
        FiniteDevice *devnode = devices->devices[ioi]; 
        if (!devnode) {
            FINITE_LOG_ERROR("Unable to read allocated memory from device list");
            continue;
        }
        if (_gamepads > 0 && gamepads != NULL) {
            for (int i = 0; i < _gamepads; i++) {
                if ( strcmp(gamepads[i]->path, devnode->path) == 0) {
                    // we already have this device, move on
                    exists = true;
                    break;
                }
            }
        }

        if (exists) {
            continue;
        }

        gamepads = realloc(gamepads, (sizeof(FiniteGamepad *) * (_gamepads + 1)));
        if (!gamepads) {
            FINITE_LOG_ERROR("Unable to allocate memory for new gamepad array");
            return false;
        }

        FiniteGamepad *current = calloc(1, sizeof(FiniteGamepad));
        if (!current) {
            FINITE_LOG_ERROR("Unable to allocate memory for new gamepad");
            return false;
        }

        int fd = open(devnode->path, O_RDONLY | O_NONBLOCK);
        if  (!fd) {
            perror("open");
            FINITE_LOG_ERROR("Found gamepad %s but unable to open it.", devnode->name);
        } else {
            FINITE_LOG("Found gamepad %s.", devnode->name);
        }

        current->fd = fd;
        current->path = strdup(devnode->path);
        current->order = shell->_gamepads; // first controller should be 0 (cuz its item 0 in the array)
        current->canInput = true; // by default all gamepads can send input
        current->lAxis = calloc(1, sizeof(FiniteJoystick));
        current->rAxis = calloc(1, sizeof(FiniteJoystick));
        current->lAxis->xAxis = ABS_X;
        current->lAxis->yAxis = ABS_Y;
        current->rAxis->xAxis = ABS_RX;
        current->rAxis->yAxis = ABS_RY;

        struct input_absinfo labsx;
        struct input_absinfo labsy;
        struct input_absinfo rabsx;
        struct input_absinfo rabsy;

        if (ioctl(fd, EVIOCGABS(current->lAxis->xAxis), &labsx) == 0) { 
            current->lAxis->xMax = labsx.maximum;
            current->lAxis->xMin = labsx.minimum;
            current->lAxis->xFlat = labsx.flat; 
        }
        if (ioctl(fd, EVIOCGABS(current->lAxis->yAxis), &labsy) == 0) {
            current->lAxis->yMax = labsy.maximum;
            current->lAxis->yMin = labsy.minimum;
            current->lAxis->yFlat = labsy.flat;
        }
        if (ioctl(fd, EVIOCGABS(current->rAxis->xAxis), &rabsx) == 0) {
            current->rAxis->xMax = rabsx.maximum;
            current->rAxis->xMin = rabsx.minimum;
            current->rAxis->xFlat = rabsx.flat;
        }
        if (ioctl(fd, EVIOCGABS(current->rAxis->yAxis), &rabsy) == 0) {
            current->rAxis->yMax = rabsy.maximum;
            current->rAxis->yMin = rabsy.minimum;
            current->rAxis->yFlat = rabsy.flat;
        }

        current->dpad = calloc(1, sizeof(FiniteDpad));
        current->dpad->xAxis = ABS_HAT0X;
        current->dpad->yAxis = ABS_HAT0Y;

        current->lt = calloc(1, sizeof(FiniteTrigger));
        current->lt->axis = ABS_HAT2Y;
        current->lt->value = 0;
        current->rt = calloc(1, sizeof(FiniteTrigger));
        current->rt->axis = ABS_HAT2X;
        current->rt->value = 0;

        current->shell = shell;


        gamepads[_gamepads] = current;
        shell->gamepads = gamepads;
        if (shell->_gamepads == 0) {
            shell->primaryGamepadId = 0;
        }
        _gamepads += 1;
        shell->_gamepads = _gamepads;




    }

    finite_free_devices(devices);

    // whether or not we have gamepads doesn't matter right now, attach the threads
    if (_gamepads > 0) {
        shell->gamepadAvailable = true;
    } else {
        shell->gamepadAvailable = false;
    } 

    shell->canInput = true;
    return true;
}

static void *finite_draw_controller_popup(void *data) {
    FiniteShell *shell = (FiniteShell *) data; // this shell has details we need to do math
    
    if (!shell) {
        FINITE_LOG_FATAL("Unable to poll gamepads with null shell.");
    }

    FiniteShell *popupShell = finite_shell_init("wayland-0");
    finite_overlay_init(popupShell, 3, "pair_device_overlay");
    double width = shell->details->width, height = shell->details->height; // this is the window size which may not always be the screen size
    double w = (width * 0.78125), h = (height * 0.648);
    
    
    finite_overlay_set_size_and_position(popupShell, w, h, ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);
    finite_overlay_set_margin(popupShell, (int)( (width - w) / 2 ) , 0, ((height - h) / 2), 0);
    finite_shm_alloc(popupShell, false);

    FiniteColorGroup bg = { 0.192, 0.192, 0.192, 1};
    FiniteColorGroup white = {1,1,1,1};
    FiniteColorGroup bar_bg = {0.262,0.262,0.262,1};
    FiniteColorGroup bar_btn_active = {1, 0.670, 0.49, 1};
    FiniteColorGroup bar_btn_inactive = {0.349,0.349,0.349, 1};
    FiniteColorGroup finite_gris0 = {0.317,0.317,0.317,1.0};// #515151

    finite_draw_rounded_rect(popupShell, 0, 0, w, h, (height * 0.02), &bg, NULL, false);    
    
    int size = (height * 0.033);
    finite_draw_set_font(popupShell, "Kumbh Sans", false, true, size);

    cairo_text_extents_t ext = finite_draw_get_text_extents(popupShell, "Pair your controllers");
    finite_draw_set_draw_position(popupShell, ((w / 2) - (ext.width / 2)), (h - (h - (ext.height * 2))));
    finite_draw_set_text(popupShell, "Pair your controllers", &white);

    cairo_surface_t *image = cairo_image_surface_create_from_png("/console/icons/controllerv2.png");
    int iw = cairo_image_surface_get_width(image), ih = cairo_image_surface_get_height(image);

    FINITE_LOG("Width: %f (%d)", w, width);
    cairo_surface_destroy(image);
    finite_draw_png(popupShell, "/console/icons/controllerv2.png", ((w / 2) - ((double) iw / 2)), ((h / 2) - (double)ih / 2), ( (double)width * 0.2119), ((double)height * 0.265), true);

    // draw status bar
    int devs = popupShell->_gamepads;
    double bw = ((double)width * 0.109), bh = (height * 0.023);
    double bx = ((w / 2) - (bw / 2)), by = (h * 0.85);
    finite_draw_rounded_rect(popupShell, bx, by, bw, bh, (height * 0.009), &bar_bg, NULL, false);

    for (int i = 0; i < 4; i++) {
        double mbw = ((double) width * 0.024), mbh = ((double) height *0.013);
        double mbx = (bx + (mbw * i) + ((width * 0.0022) * (i + 1))), mby = (by + ((bh / 2) - (mbh / 2 )));
        if (i < devs) {
            finite_draw_rounded_rect(popupShell, mbx, mby, mbw, mbh, (height * 0.005), &bar_btn_active, NULL, false);
        } else {
            finite_draw_rounded_rect(popupShell, mbx, mby, mbw, mbh, (height * 0.005), &bar_btn_inactive, NULL, false);

        }
    }

    if (devs > 0 ) {
        size = (height * 0.024); // ~26 on 1080p
        finite_draw_set_font(popupShell, "Kumbh Sans", false, true, size);
        double xW = (width * 0.176), yH = (height * 0.0629); // ~338x68
        double nW = ((double)w - (xW * 1.15)), nY = ((double)h - (yH * 1.5));
        double nRad = ((double)height * 0.03);

        finite_draw_rounded_rect(popupShell, (nW), nY, xW, yH, nRad, &finite_gris0, NULL, true);

        FiniteGradientPoint new_points[2] = {
            {0, 1, 0.325, 0.325, 1},
            {1, 1, 0.474, 0.098, 1}
        };

        cairo_pattern_t *pat = finite_draw_pattern_linear(nW, nY, (double)(width), (double)(height), new_points, 2);
        finite_draw_stroke(popupShell, NULL, pat, 7);

        cairo_pattern_destroy(pat);

        cairo_text_extents_t ext = finite_draw_get_text_extents(popupShell, "Done");
        double text_x = nW + (xW - ext.width) / 2  - ext.x_bearing;
        double text_y = nY + (yH - ext.height) / 2 - ext.y_bearing;
        
        finite_draw_set_draw_position(popupShell, (text_x) , text_y);
        finite_draw_set_text(popupShell, "Next", &white);
    }

    finite_draw_finish(popupShell, w, h, popupShell->stride, true);
    
    int state = wl_display_dispatch(popupShell->display);
    
    while (state != -1) {
        if (devs != shell->_gamepads) {
            FINITE_LOG("Shells: %d", shell->_gamepads);

            // finite_button_delete_all(popupShell);
            devs = shell->_gamepads;
            double bw = ((double)width * 0.109), bh = (height * 0.023);
            double bx = ((w / 2) - (bw / 2)), by = (h * 0.85);
            finite_draw_rounded_rect(popupShell, bx, by, bw, bh, (height * 0.009), &bar_bg, NULL, false);

            for (int i = 0; i < 4; i++) {
                double mbw = ((double) width * 0.024), mbh = ((double) height *0.013);
                double mbx = (bx + (mbw * i) + ((width * 0.0022) * (i + 1))), mby = (by + ((bh / 2) - (mbh / 2 )));
                if (i < devs) {
                    finite_draw_rounded_rect(popupShell, mbx, mby, mbw, mbh, (height * 0.005), &bar_btn_active, NULL, false);
                } else {
                    finite_draw_rounded_rect(popupShell, mbx, mby, mbw, mbh, (height * 0.005), &bar_btn_inactive, NULL, false);
                }
            }

            if (devs > 0 ) {
                size = (height * 0.024); // ~26 on 1080p
                finite_draw_set_font(popupShell, "Kumbh Sans", false, true, size);
                double xW = (width * 0.176), yH = (height * 0.0629); // ~338x68
                double nW = ((double)w - (xW * 1.15)), nY = ((double)h - (yH * 1.5));
                double nRad = ((double)height * 0.03);

                finite_draw_rounded_rect(popupShell, (nW), nY, xW, yH, nRad, &finite_gris0, NULL, true);

                FiniteGradientPoint new_points[2] = {
                    {0, 1, 0.325, 0.325, 1},
                    {1, 1, 0.474, 0.098, 1}
                };

                cairo_pattern_t *pat = finite_draw_pattern_linear(nW, nY, (double)(width), (double)(height), new_points, 2);
                finite_draw_stroke(popupShell, NULL, pat, 7);

                cairo_pattern_destroy(pat);

                cairo_text_extents_t ext = finite_draw_get_text_extents(popupShell, "Done");
                double text_x = nW + (xW - ext.width) / 2  - ext.x_bearing;
                double text_y = nY + (yH - ext.height) / 2 - ext.y_bearing;
                
                finite_draw_set_draw_position(popupShell, (text_x) , text_y);
                finite_draw_set_text(popupShell, "Next", &white);    
            } else  {
                size = (height * 0.024); // ~26 on 1080p
                finite_draw_set_font(popupShell, "Kumbh Sans", false, true, size);
                double xW = (width * 0.176), yH = (height * 0.0629); // ~338x68
                double nW = ((double)w - (xW * 1.15)), nY = ((double)h - (yH * 1.5));
                double nRad = ((double)height * 0.03);

                finite_draw_rounded_rect(popupShell, (nW), nY, xW, yH, nRad, &finite_gris0, NULL, true);
                finite_draw_stroke(popupShell, &bar_btn_inactive, NULL, 7);

                cairo_text_extents_t ext = finite_draw_get_text_extents(popupShell, "Done");
                double text_x = nW + (xW - ext.width) / 2  - ext.x_bearing;
                double text_y = nY + (yH - ext.height) / 2 - ext.y_bearing;
                
                finite_draw_set_draw_position(popupShell, (text_x) , text_y);
                finite_draw_set_text(popupShell, "Next", &bar_btn_inactive);    
            }

            finite_draw_finish(popupShell, w, h, popupShell->stride, true);
        }

        if (devs > 0 && finite_gamepad_key_pressed(0, shell, FINITE_BTN_B) ){ 
            FINITE_LOG("Close requested");
            // TODO: Animations
            break;
        }
    }

    FINITE_LOG("Done.");
    finite_draw_cleanup(popupShell);

    return data;
}

static bool finite_rescan_devices(FiniteShell *shell) {
    if (!shell) {
        FINITE_LOG_ERROR("Unable to rescan NULL shell");
        return false;
    } 

    shell->canInput = false; // discard all input events
    if (shell->_gamepads > 0){ 
        for (int i = 0; i <shell->_gamepads; i++) {
            finite_free_gamepad(shell->gamepads[i]);
        }
        shell->gamepads = NULL;
        shell->_gamepads = 0;
    }

    // TODO: allow the popup to handle this
    pthread_t thread;
    pthread_create(&thread, NULL, finite_draw_controller_popup, shell);
    pthread_detach(thread);
    // reinit
    bool success = finite_gamepad_reinit(shell);
    if (success) {
        FINITE_LOG("Found %d gamepads", shell->_gamepads);
        shell->canInput = true;
        updateRequired = true;
        return true;
    } else {
        FINITE_LOG_FATAL("Unable to rescan devices");
        return false;
    }

}

static void *finite_hotplug_devices_async(void *data) {
    FiniteShell *shell = (FiniteShell *) data;
    if (!shell) {
        FINITE_LOG_FATAL("Unable to scan for gamepads with null shell.");
    }

    int fd = inotify_init();
    
    inotify_add_watch(fd, "/dev/input", IN_CREATE | IN_DELETE | IN_ATTRIB); 

    char buf[4096];
    ssize_t len;
    while (true) {
        if (canScan) {
            len = read(fd, buf, sizeof(buf));
            if (len <= 0) {
                usleep(1000);
                continue;
            }

            for (char *p = buf; p < buf + len; ) {
                struct inotify_event *ev = (struct inotify_event *) p;
                if (ev->len && strncmp(ev->name, "event", 5) == 0) {
                    if (ev->mask & IN_CREATE) {
                        FINITE_LOG("Device added  (Gamepads %d)", shell->_gamepads);
                        finite_rescan_devices(shell);
                        break;
                    }

                    if (ev->mask & IN_DELETE) {
                        FINITE_LOG("Device removed (Gamepads %d)", shell->_gamepads);
                        finite_rescan_devices(shell);
                        break;
                    }
                }

                p += sizeof(struct inotify_event) + ev->len;
            }
        }
    }

    return data;
}

static uint16_t finite_gamepad_key_to_evdev(FiniteGamepadKey key) {
    size_t count = sizeof(finite_gamepad_key_lookup) / sizeof(finite_gamepad_key_lookup[0]);
    for (size_t i = 0; i < count; i++) {
        if (finite_gamepad_key_lookup[i].key == key) {
            return finite_gamepad_key_lookup[i].evdev_code;
        }
    }
    return UINT16_MAX; // Not found
}

static const char *finite_gamepad_evdev_to_string(uint16_t code) {
    size_t count = sizeof(finite_gamepad_key_lookup) / sizeof(finite_gamepad_key_lookup[0]);
    for (size_t i = 0; i < count; i++) {
        if (finite_gamepad_key_lookup[i].evdev_code == code) {
            return finite_gamepad_key_lookup[i].name;
        }
    }
    return ""; // Not found
}

static FiniteGamepadKey finite_gamepad_evdev_to_key(uint16_t code) {
    size_t count = sizeof(finite_gamepad_key_lookup) / sizeof(finite_gamepad_key_lookup[0]);
    for (size_t i = 0; i < count; i++) {
        if (finite_gamepad_key_lookup[i].evdev_code == code) {
            return finite_gamepad_key_lookup[i].key;
        }
    }
    return FINITE_BTN_NONE; // Not found
}

FiniteGamepadKey finite_gamepad_key_from_string_debug(const char *file, const char *func, int line, const char *name) {
    size_t count = sizeof(finite_gamepad_key_lookup) / sizeof(finite_gamepad_key_lookup[0]);
    for (size_t i = 0; i < count; i++) {
        if (strcasecmp(name, finite_gamepad_key_lookup[i].name) == 0) {
            return finite_gamepad_key_lookup[i].key;
        }
    }
    return FINITE_BTN_NONE;
}

const char *finite_gamepad_key_string_from_key_debug(const char *file, const char *func, int line, FiniteGamepadKey key) {
    size_t count = sizeof(finite_gamepad_key_lookup) / sizeof(finite_gamepad_key_lookup[0]);
    for (size_t i = 0; i < count; i++) {
        if (finite_gamepad_key_lookup[i].key == key) {
            return finite_gamepad_key_lookup[i].name;
        }
    }
    return "";
}

// Just like the finite_key api this should be called to verify a key is valid
bool finite_gamepad_key_valid_debug(const char *file, const char *func, int line, FiniteGamepadKey key) {
    return finite_gamepad_key_to_evdev(key) != UINT16_MAX;
}

bool finite_gamepad_key_down_debug(const char *file, const char *func, int line, int id, FiniteShell *shell, FiniteGamepadKey key) {
    if (!shell) {
        FINITE_LOG_ERROR("Unable to poll gamepads with null shell.");
        return false;
    }

    if (shell->canInput && id < shell->_gamepads) {
        FiniteGamepad *gamepad = shell->gamepads[id];
        if (!gamepad) {
            finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Gamepad %d is unavailable", id);
            return false;
        }
        
        FINITE_LOG("Checking if btn %s is down", finite_gamepad_key_string_from_key(key));
        if (gamepad->canInput) {
            uint16_t evdev_key = finite_gamepad_key_to_evdev(key);
            if (evdev_key == UINT16_MAX) {
                finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Unable to get input state from invalid key");
                return false;
            }
        
            return gamepad->btns[evdev_key].isDown;


        } else {
            return false;
        }

    } else {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Gamepad %d is unavailable", id);
        return false;
    }

}

bool finite_gamepad_key_up_debug(const char *file, const char *func, int line, int id, FiniteShell *shell, FiniteGamepadKey key) {
    if (!shell) {
        FINITE_LOG_ERROR("Unable to poll gamepads with null shell.");
        return false;
    }

    if (shell->canInput && id < shell->_gamepads) {
        FiniteGamepad *gamepad = shell->gamepads[id];
        if (!gamepad) {
            finite_log_internal(LOG_LEVEL_WARN, file, line, func, "Gamepad %d is unavailable", id);
            return false;
        }
        
        FINITE_LOG("Checking if btn %s is up", finite_gamepad_key_string_from_key(key));
        if (gamepad->canInput) {
            uint16_t evdev_key = finite_gamepad_key_to_evdev(key);
            if (evdev_key == UINT16_MAX) {
                finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Unable to get input state from invalid key");
                return false;
            }
        
            return gamepad->btns[evdev_key].isUp;


        } else {
            return false;
        }

    } else {
        finite_log_internal(LOG_LEVEL_WARN, file, line, func, "Gamepad %d is unavailable", id);
        return false;
    }
}

bool finite_gamepad_key_pressed_debug(const char *file, const char *func, int line, int id, FiniteShell *shell,  FiniteGamepadKey key) {
    if (!shell) {
        FINITE_LOG_ERROR("Unable to poll gamepads with null shell.");
        return false;
    }


    if (shell->canInput && id < shell->_gamepads) {
        FiniteGamepad *gamepad = shell->gamepads[id];
        if (!gamepad) {
            finite_log_internal(LOG_LEVEL_WARN, file, line, func, "Gamepad %d is unavailable", id);
            return false;
        }

        FINITE_LOG("Checking if btn %s was pressed", finite_gamepad_key_string_from_key(key));
        if (gamepad->canInput) {
            uint16_t evdev_key = finite_gamepad_key_to_evdev(key);
            if (evdev_key == UINT16_MAX) {
                finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Unable to get input state from invalid key");
                return false;
            }

            if (gamepad->btns[evdev_key].isDown && !gamepad->btns[evdev_key].isHeld) {
                gamepad->btns[evdev_key].isHeld = true;
            }

            if (gamepad->btns[evdev_key].isUp && gamepad->btns[evdev_key].isHeld) {
                gamepad->btns[evdev_key].isHeld = false;
                return true;
            } else {
                return false;
            }
        
        } else {
            finite_log_internal(LOG_LEVEL_WARN, file, line, func, "Gamepad %d is unavailable", id);
            return false;
        }
    } else {
        finite_log_internal(LOG_LEVEL_WARN, file, line, func, "Gamepad %d is unavailable", id);
        return false;
    }
}

double finite_gamepad_joystick_get_value_debug(const char *file, const char *func, int line, int id, FiniteShell *shell, FiniteJoystickType type ) {
    if (!shell) {
        FINITE_LOG_ERROR("Unable to poll gamepads with null shell.");
        return false;
    }


    if (shell->canInput && id < shell->_gamepads) {
        FiniteGamepad *gamepad = shell->gamepads[id];

        if (!gamepad) {
            finite_log_internal(LOG_LEVEL_WARN, file, line, func, "Gamepad %d is unavailable", id);
            return (double) 0;
        }

        if (type == FINITE_JOYSTICK_LEFT_X) {
            return gamepad->lAxis->xValue;
        } else if (type == FINITE_JOYSTICK_LEFT_Y) {
            return gamepad->lAxis->yValue;
        } else if (type == FINITE_JOYSTICK_RIGHT_X) {
            return gamepad->rAxis->xValue;
        } else if (type == FINITE_JOYSTICK_RIGHT_Y) {
            return gamepad->rAxis->yValue;
        } else {
            return (double) 0;
        }
    } else {
        finite_log_internal(LOG_LEVEL_WARN, file, line, func, "Gamepad %d is unavailable", id);
        return (double) 0;

    }
}

// draw the No_Home popup and then destroy it after 3 seconds
static void *finite_gamepad_no_way_home(void *data) {
    FiniteShell *shell = (FiniteShell *) data; // this shell has details we need to do math
    if (!shell) {
        FINITE_LOG_FATAL("Unable to poll gamepads with null shell.");
    }

    FiniteShell *popupShell = finite_shell_init("wayland-0");
    finite_overlay_init(popupShell, 3, "overlay");
    double width = shell->details->width, height = shell->details->height; // this is the window size which may not always be the screen size
    finite_overlay_set_size_and_position(popupShell, (width * 0.067), (height * 0.118), ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);

    finite_shm_alloc(popupShell, false);

    cairo_surface_t *img = cairo_image_surface_create_from_png("/console/icons/input/no_home.png");
    if (cairo_surface_status(img) == CAIRO_STATUS_SUCCESS) {
        int w = cairo_image_surface_get_width(img), h = cairo_image_surface_get_height(img);

        finite_draw_rect(popupShell, 0, 0,  (width * 0.067), (height * 0.118), NULL, NULL);

        cairo_save(popupShell->cr);
        cairo_clip(popupShell->cr);
        cairo_new_path(popupShell->cr);
        cairo_translate(popupShell->cr, 0,0);
        
        cairo_scale(popupShell->cr, (width * 0.067)/w, (height * 0.118)/h);
        cairo_set_source_surface(popupShell->cr, img, 0,0);
        cairo_paint(popupShell->cr);
        cairo_surface_destroy(img);

        cairo_restore(popupShell->cr);
    } else {
        FINITE_LOG_WARN("Unable to load image: %s", cairo_status_to_string(cairo_surface_status(img)));
    }

    bool success = finite_draw_finish(popupShell, (width * 0.067), (height * 0.118), popupShell->stride, true);
    if (!success) {
        FINITE_LOG_ERROR("Something went wrong.\n");
        free(popupShell);
    }

    sleep(3);
    FINITE_LOG("Done.");
    finite_draw_cleanup(popupShell);

    return data;
}

static void *finite_gamepad_poll_buttons_async(void *data) {
    FiniteShell *shell = (FiniteShell *) data;
    if (!shell) {
        FINITE_LOG_FATAL("Unable to poll gamepads with null shell.");
    }

    FINITE_LOG("Polling started...");

    FiniteGamepad **gamepads = NULL;
    int _gamepads = 0;
    if (shell->_gamepads > 0) {
        _gamepads = shell->_gamepads;
        FINITE_LOG("Attempting to copy data into gamepads");
        FiniteGamepad **tmp = malloc(sizeof(FiniteGamepad *) * _gamepads);
        if (!tmp) {
            FINITE_LOG_FATAL("Unable to malloc required memory for polling");
        } else {
            gamepads = tmp;
        }
        FINITE_LOG("Attempting to copy gamepad data for polling.");
        memcpy(gamepads, shell->gamepads, sizeof(FiniteGamepad *) * _gamepads); // only memcpy when its safe to do so
        FINITE_LOG("Copied succesfully");
    }

    while (true) {
        if (shell->canInput) {
            if (updateRequired) {
                free(gamepads);
                gamepads = NULL;
                _gamepads = 0;
                if (shell->_gamepads > 0) {
                    _gamepads = shell->_gamepads;
                    FINITE_LOG("Attempting to recopy data into gamepads");
                    FiniteGamepad **tmp = malloc(sizeof(FiniteGamepad *) * _gamepads);
                    if (!tmp) {
                        FINITE_LOG_FATAL("Unable to malloc required memory for polling");
                    } else {
                        gamepads = tmp;
                    }
                    FINITE_LOG("Attempting to recopy gamepad data for polling.");
                    memcpy(gamepads, shell->gamepads, sizeof(FiniteGamepad *) * _gamepads); // only memcpy when its safe to do so
                    FINITE_LOG("Copied succesfully");
                    updateRequired = false;
                } else {
                    updateRequired = false;
                }

            }

            if (_gamepads > 0) {
                for (int g = 0; g < _gamepads ; g++) {
                    FiniteGamepad *current = gamepads[g];
                    struct input_event ev;

                    if (current->fd <= 0) {
                        FINITE_LOG_WARN("Gamepad %d has an invalid file descriptor and cannot be read from.");

                        break; 
                    }

                    ssize_t d = read(current->fd, &ev,  sizeof(struct input_event));

                    if (d > 0 ) {
                        
                        FINITE_LOG_INFO("ev: type=%d, code=%d (%s), value=%d", ev.type, ev.code, finite_gamepad_evdev_to_string(ev.code),  ev.value);
                        if (ev.type == EV_KEY) {
                            FINITE_LOG_INFO("Key %s pressed", finite_gamepad_evdev_to_string(ev.code));
                            current->btns[ev.code].isDown = (ev.value == 1) ? true : false;
                            current->btns[ev.code].isUp = (ev.value == 0) ? true : false;

                            if (finite_gamepad_key_pressed(g, shell, FINITE_BTN_HOME) ) {
                                FINITE_LOG("Home Button pressed");
                                // TODO: allow games to disable shortcuts
                                if (shell->canHomeMenu) {
                                    FINITE_LOG("Home menu requested.");
                                    // TODO: close anim
                                } else {
                                    pthread_t noWayHome;
                                    pthread_create(&noWayHome, NULL, finite_gamepad_no_way_home, shell);
                                    pthread_detach(noWayHome);
                                }
                            }

                            if (finite_gamepad_key_pressed(g, shell, FINITE_BTN_A) ) {
                                finite_button_handle_poll(FINITE_DIRECTION_DONE, shell);
                            }


                        } else if (ev.type == EV_ABS) {
                            if (ev.code == current->dpad->xAxis) {
                                if (ev.value < 0) {
                                    FINITE_LOG_INFO("Key %s pressed", finite_gamepad_key_string_from_key(FINITE_BTN_LEFT));
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_LEFT)].isDown = true;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_LEFT)].isUp = false;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_RIGHT)].isDown = false;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_RIGHT)].isUp = true;
                                    current->dpad->xValue = ev.value;
                                    finite_button_handle_poll(FINITE_DIRECTION_LEFT, shell);

                                } else if (ev.value > 0) {
                                    FINITE_LOG_INFO("Key %s pressed", finite_gamepad_key_string_from_key(FINITE_BTN_RIGHT));
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_LEFT)].isDown = false;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_LEFT)].isUp = true;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_RIGHT)].isDown = true;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_RIGHT)].isUp = false;
                                    current->dpad->xValue = ev.value;
                                    finite_button_handle_poll(FINITE_DIRECTION_RIGHT, shell);

                                } else {
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_LEFT)].isDown = false;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_LEFT)].isUp = true;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_RIGHT)].isDown = false;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_RIGHT)].isUp = true;
                                    current->dpad->xValue = ev.value;
                                }
                            } else if (ev.code == current->dpad->yAxis) {
                                if (ev.value < 0) {
                                    FINITE_LOG_INFO("Key %s pressed", finite_gamepad_key_string_from_key(FINITE_BTN_UP));
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_UP)].isDown = true;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_UP)].isUp = false;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_DOWN)].isDown = false;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_DOWN)].isUp = true;
                                    current->dpad->yValue = ev.value;
                                } else if (ev.value > 0) {
                                    FINITE_LOG_INFO("Key %s pressed", finite_gamepad_key_string_from_key(FINITE_BTN_DOWN));
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_UP)].isDown = false;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_UP)].isUp = true;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_DOWN)].isDown = true;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_DOWN)].isUp = false;
                                    current->dpad->yValue = ev.value;
                                    finite_button_handle_poll(FINITE_DIRECTION_DOWN, shell);

                                } else {
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_UP)].isDown = false;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_UP)].isUp = true;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_DOWN)].isDown = false;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_DOWN)].isUp = true;
                                    current->dpad->yValue = ev.value;
                                    finite_button_handle_poll(FINITE_DIRECTION_UP, shell);

                                }
                            } else if (ev.code == current->lt->axis) {
                                    FINITE_LOG_INFO("Key %s event", finite_gamepad_key_string_from_key(FINITE_BTN_LEFT_TRIGGER));

                                if (ev.value > 0) {
                                    FINITE_LOG_INFO("Key %s pressed", finite_gamepad_key_string_from_key(FINITE_BTN_LEFT_TRIGGER));
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_LEFT_TRIGGER)].isDown = true;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_LEFT_TRIGGER)].isUp = false;
                                    current->lt->value = ev.value;
                                } else {
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_LEFT_TRIGGER)].isDown = false;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_LEFT_TRIGGER)].isUp = true;
                                    current->lt->value = ev.value;

                                }
                            } else if (ev.code == current->rt->axis) {
                                if (ev.value > 0) {
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_RIGHT_TRIGGER)].isDown = true;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_RIGHT_TRIGGER)].isUp = false;
                                    current->rt->value = ev.value;

                                } else {
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_RIGHT_TRIGGER)].isDown = false;
                                    current->btns[finite_gamepad_key_to_evdev(FINITE_BTN_RIGHT_TRIGGER)].isUp = true;
                                    current->rt->value = ev.value;
                                }
                            } else if (ev.code == current->lAxis->xAxis) {
                                // do math here
                                double norm;

                                int center = (current->lAxis->xMax + current->lAxis->xMin) / 2;
                                if (current->lAxis->xFlat > 0 && abs(ev.value - center) < current->lAxis->xFlat) {
                                    norm = 0.0;
                                } else {
                                    norm = (2.0 * (ev.value - current->lAxis->xMin) / (current->lAxis->xMax - current->lAxis->xMin)) - 1.0;
                                }
                                current->lAxis->xValue = norm;
                            } else if (ev.code == current->lAxis->yAxis) {
                                // do math here
                                double norm;

                                int center = (current->lAxis->yMax + current->lAxis->yMin) / 2;
                                if (current->lAxis->yFlat > 0 && abs(ev.value - center) < current->lAxis->yFlat) {
                                    norm = 0.0;
                                } else {
                                    norm = (2.0 * (ev.value - current->lAxis->yMin) / (current->lAxis->yMax - current->lAxis->yMin)) - 1.0;
                                }

                                current->lAxis->yValue = norm;
                            } else if (ev.code == current->rAxis->xAxis) {
                                // do math here
                                double norm;

                                int center = (current->rAxis->xMax + current->rAxis->xMin) / 2;
                                if (current->rAxis->xFlat > 0 && abs(ev.value - center) < current->rAxis->xFlat) {
                                    norm = 0.0;
                                } else {
                                    norm = (2.0 * (ev.value - current->rAxis->xMin) / (current->rAxis->xMax - current->rAxis->xMin)) - 1.0;
                                }

                                current->rAxis->xValue = norm;
                            } else if (ev.code == current->rAxis->yAxis) {
                                // do math here
                                double norm;

                                int center = (current->rAxis->yMax + current->rAxis->yMin) / 2;
                                if (current->rAxis->yFlat > 0 && abs(ev.value - center) < current->rAxis->yFlat) {
                                    norm = 0.0;
                                } else {
                                    norm = (2.0 * (ev.value - current->rAxis->yMin) / (current->rAxis->yMax - current->rAxis->yMin)) - 1.0;
                                }

                                current->rAxis->yValue = norm;
                            }
                        }
                    } else {
                        if ((d < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
                            // force a rescan
                            if (errno == ENODEV || errno == ENOENT || errno == EBADF) {
                                // device removed
                                break;
                            } else {
                                perror("read");
                                break;
                            }
                        }

                        continue;
                    }
                }
            }
        }
        usleep(1000);
    }

    return shell;

}

