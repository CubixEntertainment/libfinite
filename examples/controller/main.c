/*
    Window drawing with Libfinite SDK example
    Written by Gabriel Thompson <gabriel.thomp@cubixdev.org>
*/

#include "draw/cairo.h"
#include "draw/window.h"
#include "input/gamepad.h"
#include <finite/draw.h>
#include <finite/log.h>
#include <finite/input.h>
#include <linux/limits.h>
#include <string.h>

FiniteShell *myShell;
int32_t width;
int32_t height;

int _showKeys = 11;
FiniteGamepadKey showKeys[] = {
        // letters
    FINITE_BTN_A,
    FINITE_BTN_B,
    FINITE_BTN_X,
    FINITE_BTN_Y,

    // Triggers
    FINITE_BTN_RIGHT_SHOULDER,
    FINITE_BTN_RIGHT_TRIGGER,
    FINITE_BTN_LEFT_SHOULDER,
    FINITE_BTN_LEFT_TRIGGER,
    FINITE_BTN_START,
    FINITE_BTN_SELECT,
    FINITE_BTN_HOME
};

// dpad has special requirements

void draw_controller() {
    if (myShell->gamepadAvailable) {
        const char basePath[] = "/console/icons/input/"; // at some point we'll make an official icon set

        // clear layer
        FiniteColorGroup white = {0.067,0.067,0.067,1.0};

        finite_draw_rect(myShell, 0,0, width, height, &white, NULL);

        double x,y;
        double image_width = (width * 0.033), image_height = (height * 0.059);
        int lines = 1;

        for (int s = 0; s < _showKeys; s++) {
            char fullPath[PATH_MAX];
            strcpy(fullPath, basePath);
            if (finite_gamepad_key_down(0, myShell, showKeys[s])) {
                strcat(fullPath, finite_gamepad_key_string_from_key(showKeys[s]));
                strcat(fullPath, ".png");
            } else {
                strcat(fullPath, finite_gamepad_key_string_from_key(showKeys[s]));
                strcat(fullPath, "_Fill.png");
            }

            FINITE_LOG_INFO("Path: %s", fullPath);
        
            
            x = (((width * 0.2) + (image_width * s) ) - (image_width / 2));
            y = ((height * 0.25) - (image_height / 2));
            if (x > (width - (width *0.2))) {
                y = (((height * 0.25) + (image_height * 1.25)) - (image_height / 2));
                lines++;
            }

            finite_draw_png(myShell, fullPath, x, y, image_width, image_height, true);
        }

        // handle dpad here
        char fullPath[PATH_MAX];
        strcpy(fullPath, basePath);
        if (finite_gamepad_key_down(0, myShell, FINITE_BTN_DOWN)) {
            strcat(fullPath, finite_gamepad_key_string_from_key(FINITE_BTN_DOWN));
            strcat(fullPath, ".png");
        } else if (finite_gamepad_key_down(0, myShell, FINITE_BTN_UP)) {
            strcat(fullPath, finite_gamepad_key_string_from_key(FINITE_BTN_UP));
            strcat(fullPath, ".png");
        } else if (finite_gamepad_key_down(0, myShell, FINITE_BTN_LEFT)) {
            strcat(fullPath, finite_gamepad_key_string_from_key(FINITE_BTN_LEFT));
            strcat(fullPath, ".png");
        } else if (finite_gamepad_key_down(0, myShell, FINITE_BTN_RIGHT)) {
            strcat(fullPath, finite_gamepad_key_string_from_key(FINITE_BTN_RIGHT));
            strcat(fullPath, ".png");
        } else {
            strcat(fullPath, "Dpad.png");
        }

        // FINITE_LOG_INFO("Path: %s", fullPath);

        x = ((width * 0.2) - (image_width / 2));
        y = (((height * 0.25) + ((image_height * 1.25) * lines)) - (image_height/ 2));
        
        finite_draw_png(myShell, fullPath, x, y, image_width, image_height, true);
        lines++;

        // handle joystick here

        // LStick Horizontal
        memset(fullPath, 0, sizeof(fullPath));
        strcpy(fullPath, basePath);

        double axis = finite_gamepad_joystick_get_value(0, myShell, FINITE_JOYSTICK_LEFT_X);

        if (finite_gamepad_key_down(0, myShell, FINITE_BTN_LEFT_JOYSTICK)) {
            strcat(fullPath, finite_gamepad_key_string_from_key(FINITE_BTN_LEFT_JOYSTICK));
            strcat(fullPath, ".png");
        } else if (axis > 0) {
            strcat(fullPath, "LStickRight.png");
        } else if (axis < 0) {
            strcat(fullPath, "LStickLeft.png");
        } else {
            strcat(fullPath, "LStick.png");
        }

        x = ((width * 0.2) - (image_width / 2));
        y = (((height * 0.25) + ((image_height * 1.25) * lines)) - (image_height/ 2));
        
        finite_draw_png(myShell, fullPath, x, y, image_width, image_height, true);

        // LStick vertical

        memset(fullPath, 0, sizeof(fullPath));
        strcpy(fullPath, basePath);

        axis = finite_gamepad_joystick_get_value(0, myShell, FINITE_JOYSTICK_LEFT_Y);

        if (axis > 0) {
            strcat(fullPath, "LStickDown.png");
        } else if (axis < 0) {
            strcat(fullPath, "LStickUp.png");
        } else {
            strcat(fullPath, "LStick.png");
        }

        x = ((width * 0.2) + image_width - (image_width / 2));
        finite_draw_png(myShell, fullPath, x, y, image_width, image_height, true);
        lines++;


        // RStick Horizontal
        memset(fullPath, 0, sizeof(fullPath));
        strcpy(fullPath, basePath);
    
        axis = finite_gamepad_joystick_get_value(0, myShell, FINITE_JOYSTICK_RIGHT_X);

        if (finite_gamepad_key_down(0, myShell, FINITE_BTN_RIGHT_JOYSTICK)) {
            strcat(fullPath, finite_gamepad_key_string_from_key(FINITE_BTN_RIGHT_JOYSTICK));
            strcat(fullPath, ".png");
        } else if (axis > 0) {
            strcat(fullPath, "RStickRight.png");
        } else if (axis < 0) {
            strcat(fullPath, "RStickLeft.png");
        } else {
            strcat(fullPath, "RStick.png");
        }

        x = ((width * 0.2) - (image_width / 2));
        y = (((height * 0.25) + ((image_height * 1.25) * lines)) - (image_height/ 2));
        
        finite_draw_png(myShell, fullPath, x, y, image_width, image_height, true);

        // RStick vertical

        memset(fullPath, 0, sizeof(fullPath));
        strcpy(fullPath, basePath);
        axis = finite_gamepad_joystick_get_value(0, myShell, FINITE_JOYSTICK_RIGHT_Y);

        if (axis > 0) {
            strcat(fullPath, "RStickDown.png");
        } else if (axis < 0) {
            strcat(fullPath, "RStickUp.png");
        } else {
            strcat(fullPath, "RStick.png");
        }

        x = ((width * 0.2) + image_width - (image_width / 2));
        finite_draw_png(myShell, fullPath, x, y, image_width, image_height, true);

        bool success = finite_draw_finish(myShell, width, height, myShell->stride, true);
        if (!success) {
            FINITE_LOG_ERROR("Something went wrong.\n");
            free(myShell);
        }
    } else {
        // clear layer
        FiniteColorGroup white = {0.067,0.067,0.067,1.0};

        finite_draw_rect(myShell, 0,0, width, height, &white, NULL);
        FINITE_LOG_INFO("Path: %s", "/console/icons/input/no_controller.png");
        double image_width = (width * 0.067), image_height = (height * 0.118);
        int x = ((width * 0.5) - (image_width / 2)), y = ((height * 0.5) - (image_height / 2));
        finite_draw_png(myShell, "/console/icons/input/no_controller.png", x, y, image_width, image_height, true);


        bool success = finite_draw_finish(myShell, width, height, myShell->stride, true);
        if (!success) {
            FINITE_LOG_ERROR("Something went wrong.\n");
            free(myShell);
        }
    }
}

int main() {
    finite_log_init(stdout, LOG_LEVEL_DEBUG, false);
    myShell = finite_shell_init("wayland-0");
    
    if (!myShell) {
        FINITE_LOG("Unable to init shell");
        return 1;
    }

    finite_window_init(myShell);

    if (!myShell) {
        FINITE_LOG("Unable to make shell");
        return 1;
    }

    FiniteWindowInfo *det = myShell->details;
    width = det->width;
    height = det->height;
    finite_shm_alloc(myShell, true);
    FiniteColorGroup white = {0.067,0.067,0.067,1.0};
    finite_draw_rect(myShell, 0,0, width, height, &white, NULL);

    bool success = finite_draw_finish(myShell, width, height, myShell->stride, true);
    if (!success) {
        FINITE_LOG_ERROR("Something went wrong.\n");
        free(myShell);
    }


    // ensure a gamepad is connected
    bool withGP = finite_gamepad_init(myShell);
    myShell->canHomeMenu = false; // disable home memu

    if (!withGP) {
        // this will only be false if the device is unable to search for a gamepad.
        // ? otherwise you should check if shell.gamepadAvailable is true before doing anything controller dependent.
        FINITE_LOG_ERROR("Can't poll controller data");
    }

    // now just keep the window alive
    while (wl_display_dispatch(myShell->display) != -1) {
        draw_controller();
    }
    finite_draw_cleanup(myShell);    
    wl_display_disconnect(myShell->display);
}
