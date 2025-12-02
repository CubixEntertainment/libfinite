#include "include/input.h"
#include "include/draw/window.h"
#include "include/log.h"

// create a registry_listener struct for future use
const struct wl_registry_listener key_registry_listener = {
    .global = islands_key_registry_handle,
    .global_remove = islands_key_registry_handle_remove
};

const struct wl_keyboard_listener keys_listener = {
    .key = islands_key_handle,
    .modifiers = islands_key_mod_handle,
    .keymap = islands_key_map_handle,
    .enter = islands_key_enter_handle,
    .leave = islands_key_leave_handler,
    .repeat_info = NULL
};

const FiniteKeyMapping finite_key_lookup[] = {
    // Letters
    { "A", FINITE_KEY_A, KEY_A }, { "B", FINITE_KEY_B, KEY_B }, { "C", FINITE_KEY_C, KEY_C },
    { "D", FINITE_KEY_D, KEY_D }, { "E", FINITE_KEY_E, KEY_E }, { "F", FINITE_KEY_F, KEY_F },
    { "G", FINITE_KEY_G, KEY_G }, { "H", FINITE_KEY_H, KEY_H }, { "I", FINITE_KEY_I, KEY_I },
    { "J", FINITE_KEY_J, KEY_J }, { "K", FINITE_KEY_K, KEY_K }, { "L", FINITE_KEY_L, KEY_L },
    { "M", FINITE_KEY_M, KEY_M }, { "N", FINITE_KEY_N, KEY_N }, { "O", FINITE_KEY_O, KEY_O },
    { "P", FINITE_KEY_P, KEY_P }, { "Q", FINITE_KEY_Q, KEY_Q }, { "R", FINITE_KEY_R, KEY_R },
    { "S", FINITE_KEY_S, KEY_S }, { "T", FINITE_KEY_T, KEY_T }, { "U", FINITE_KEY_U, KEY_U },
    { "V", FINITE_KEY_V, KEY_V }, { "W", FINITE_KEY_W, KEY_W }, { "X", FINITE_KEY_X, KEY_X },
    { "Y", FINITE_KEY_Y, KEY_Y }, { "Z", FINITE_KEY_Z, KEY_Z },

    // Digits
    { "0", FINITE_KEY_0, KEY_0 }, { "1", FINITE_KEY_1, KEY_1 }, { "2", FINITE_KEY_2,KEY_2 },
    { "3", FINITE_KEY_3, KEY_3 }, { "4", FINITE_KEY_4, KEY_4 }, { "5", FINITE_KEY_5, KEY_5 },
    { "6", FINITE_KEY_6, KEY_6 }, { "7", FINITE_KEY_7, KEY_7 }, { "8", FINITE_KEY_8, KEY_8 },
    { "9", FINITE_KEY_9, KEY_9 },

    // Modifiers
    { "LeftShift",  FINITE_KEY_LEFT_SHIFT, KEY_LEFTSHIFT },   { "RightShift", FINITE_KEY_RIGHT_SHIFT, KEY_RIGHTSHIFT },
    { "LeftCtrl",   FINITE_KEY_LEFT_CTRL, KEY_LEFTCTRL },    { "RightCtrl",  FINITE_KEY_RIGHT_CTRL, KEY_RIGHTCTRL },
    { "LeftAlt",    FINITE_KEY_LEFT_ALT, KEY_LEFTALT },     { "RightAlt",   FINITE_KEY_RIGHT_ALT, KEY_RIGHTALT },
    { "LeftMeta",   FINITE_KEY_LEFT_META, KEY_LEFTMETA },    { "RightMeta",  FINITE_KEY_RIGHT_META, KEY_RIGHTMETA },

    // Arrows
    { "Up",    FINITE_KEY_UP, KEY_UP },
    { "Down",  FINITE_KEY_DOWN, KEY_DOWN },
    { "Left",  FINITE_KEY_LEFT, KEY_LEFT },
    { "Right", FINITE_KEY_RIGHT, KEY_RIGHT },

    // Function keys
    { "F1", FINITE_KEY_F1, KEY_F1 }, { "F2", FINITE_KEY_F2, KEY_F2 }, { "F3", FINITE_KEY_F3, KEY_F3 },
    { "F4", FINITE_KEY_F4, KEY_F4 }, { "F5", FINITE_KEY_F5, KEY_F5 }, { "F6", FINITE_KEY_F6, KEY_F6 },
    { "F7", FINITE_KEY_F7, KEY_F7 }, { "F8", FINITE_KEY_F8, KEY_F8 }, { "F9", FINITE_KEY_F9, KEY_F9 },
    { "F10", FINITE_KEY_F10, KEY_F10 }, { "F11", FINITE_KEY_F11, KEY_F11 }, { "F12", FINITE_KEY_F12, KEY_F12 },

    // Editing / navigation
    { "Enter",      FINITE_KEY_ENTER, KEY_ENTER },
    { "Escape",     FINITE_KEY_ESCAPE, KEY_ESC },
    { "Backspace",  FINITE_KEY_BACKSPACE, KEY_BACKSPACE },
    { "Tab",        FINITE_KEY_TAB, KEY_TAB },
    { "Space",      FINITE_KEY_SPACE, KEY_SPACE },
    { "CapsLock",   FINITE_KEY_CAPS_LOCK, KEY_CAPSLOCK },
    { "Insert",     FINITE_KEY_INSERT, KEY_INSERT },
    { "Delete",     FINITE_KEY_DELETE, KEY_DELETE },
    { "Home",       FINITE_KEY_HOME, KEY_HOME },
    { "End",        FINITE_KEY_END, KEY_END },
    { "PageUp",     FINITE_KEY_PAGE_UP, KEY_PAGEUP },
    { "PageDown",   FINITE_KEY_PAGE_DOWN, KEY_PAGEDOWN },

    // Meta
    { "PrintScreen", FINITE_KEY_PRINT_SCREEN, KEY_PRINT },
    { "ScrollLock",  FINITE_KEY_SCROLL_LOCK, KEY_SCROLLLOCK },
    { "Pause",       FINITE_KEY_PAUSE, KEY_PAUSE },

    // Symbols and punctuation
    { "Minus",        FINITE_KEY_MINUS, KEY_MINUS },
    { "Equals",       FINITE_KEY_EQUALS, KEY_EQUAL },
    { "LeftBracket",  FINITE_KEY_LEFT_BRACKET, KEY_LEFTBRACE },
    { "RightBracket", FINITE_KEY_RIGHT_BRACKET, KEY_RIGHTBRACE },
    { "Backslash",    FINITE_KEY_BACKSLASH, KEY_BACKSLASH },
    { "Semicolon",    FINITE_KEY_SEMICOLON, KEY_SEMICOLON },
    { "Apostrophe",   FINITE_KEY_APOSTROPHE, KEY_APOSTROPHE },
    { "Grave",        FINITE_KEY_GRAVE, KEY_GRAVE },
    { "Comma",        FINITE_KEY_COMMA, KEY_COMMA },
    { "Period",       FINITE_KEY_PERIOD, KEY_DOT },
    { "Slash",        FINITE_KEY_SLASH, KEY_SLASH },

    // Numpad
    { "NumLock",      FINITE_KEY_NUM_LOCK, KEY_NUMLOCK },
    { "KpDivide",     FINITE_KEY_KP_DIVIDE, KEY_KPSLASH },
    { "KpMultiply",   FINITE_KEY_KP_MULTIPLY, KEY_KPASTERISK },
    { "KpMinus",      FINITE_KEY_KP_MINUS, KEY_KPMINUS },
    { "KpPlus",       FINITE_KEY_KP_PLUS, KEY_KPPLUS },
    { "KpEnter",      FINITE_KEY_KP_ENTER, KEY_KPENTER },
    { "Kp0",          FINITE_KEY_KP_0, KEY_KP0 },
    { "Kp1",          FINITE_KEY_KP_1, KEY_KP1 },
    { "Kp2",          FINITE_KEY_KP_2, KEY_KP2 },
    { "Kp3",          FINITE_KEY_KP_3, KEY_KP3 },
    { "Kp4",          FINITE_KEY_KP_4, KEY_KP4 },
    { "Kp5",          FINITE_KEY_KP_5, KEY_KP5 },
    { "Kp6",          FINITE_KEY_KP_6, KEY_KP6 },
    { "Kp7",          FINITE_KEY_KP_7, KEY_KP7 },
    { "Kp8",          FINITE_KEY_KP_8, KEY_KP8 },
    { "Kp9",          FINITE_KEY_KP_9, KEY_KP9 },
    { "KpPeriod",     FINITE_KEY_KP_PERIOD, KEY_KPDOT },
};


FiniteKeyboard *finite_input_keyboard_init_debug(const char *file, const char *func, int line, struct wl_display *device) {
    FiniteKeyboard *keyboard = calloc(1, sizeof(FiniteKeyboard));
    keyboard->input = calloc(1, sizeof(FiniteInput));
    FiniteInput *input = keyboard->input;

    input->display = device;
    if (!input->display) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Unable to attach to display");
        free(input);
        return NULL;
    }

    input->registry = wl_display_get_registry(input->display);
    if (!input->registry) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Unable to attach to registry");
        free(input);
        return NULL;
    }

    wl_registry_add_listener(input->registry, &key_registry_listener, input);
    wl_display_roundtrip(input->display);

    if (!input->isle) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Unable to find a compositor.");
        free(input);
        return NULL;
    }

    if (!input->seat) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Unable to find a wl_seat.");
        free(input);
        return NULL;
    }

    keyboard->keyboard = wl_seat_get_keyboard(keyboard->input->seat);
    if (!keyboard->keyboard) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Unable to find a wl_seat.");
        free(input);
        free(keyboard);
        return NULL;
    }

    wl_keyboard_add_listener(keyboard->keyboard, &keys_listener, keyboard);
    return keyboard;
}

static uint16_t finite_key_to_evdev(FiniteKey key) {
    size_t count = sizeof(finite_key_lookup) / sizeof(finite_key_lookup[0]);
    for (size_t i = 0; i < count; i++) {
        if (finite_key_lookup[i].key == key) {
            return finite_key_lookup[i].evdev_code;
        }
    }
    return UINT16_MAX; // Not found
}

bool finite_key_valid_debug(const char *file, const char *func, int line, FiniteKey key) {
    return finite_key_to_evdev(key) != UINT16_MAX;
}

// ALWAYS check the that the key is valid with finite_key_valid
bool finite_key_down_debug(const char *file, const char *func, int line, FiniteKey key, FiniteKeyboard *board) {
    if (!board) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Unable to get input from NULL device");
        return false;
    }
    if (key == FINITE_KEY_INVALID) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Unable to get input from Invalid key");
        return false;
    }

    uint16_t evdev_key = finite_key_to_evdev(key);
    if (evdev_key == UINT16_MAX) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Unable to get input from Invalid key");
        return false;
    }

    return board->keys[evdev_key].isDown;
}

// ALWAYS check the that the key is valid with finite_key_valid
bool finite_key_up_debug(const char *file, const char *func, int line, FiniteKey key, FiniteKeyboard *board) {
    if (!board) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Unable to get input from NULL device");
        return false;
    }
    if (key == FINITE_KEY_INVALID) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Unable to get input from Invalid key");
        return false;
    }

    uint16_t evdev_key = finite_key_to_evdev(key);
    if (evdev_key == UINT16_MAX) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Unable to get input from Invalid key");
        return false;
    }

    return board->keys[evdev_key].isUp;
}

// ALWAYS check the that the key is valid with finite_key_valid
bool finite_key_pressed_debug(const char *file, const char *func, int line, FiniteKey key, FiniteKeyboard *board) {
    if (!board) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Unable to get input from NULL device");
        return false;
    }
    if (key == FINITE_KEY_INVALID) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Unable to get input from Invalid key");
        return false;
    }

    uint16_t evdev_key = finite_key_to_evdev(key);
    if (evdev_key == UINT16_MAX) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Unable to get input from Invalid key");
        return false;
    }  

    if (board->keys[evdev_key].isDown && !board->keys[evdev_key].isHeld) {
        // then set isHeld to true
        board->keys[evdev_key].isHeld = true;
    }

    if (board->keys[evdev_key].isUp && board->keys[evdev_key].isHeld) {
        // then set isHeld to false
        board->keys[evdev_key].isHeld = false;
        return true;
    }

    return false;
}

FiniteKey finite_key_from_string_debug(const char *file, const char *func, int line, const char *name) {
    size_t count = sizeof(finite_key_lookup) / sizeof(finite_key_lookup[0]);
    for (size_t i = 0; i < count; i++) {
        if (strcasecmp(name, finite_key_lookup[i].name) == 0) {
            return finite_key_lookup[i].key;
        }
    }
    return FINITE_KEY_INVALID;
}


const char *finite_key_string_from_key_debug(const char *file, const char *func, int line, FiniteKey key) {
    size_t count = sizeof(finite_key_lookup) / sizeof(finite_key_lookup[0]);
    for (size_t i = 0; i < count; i++) {
        if (finite_key_lookup[i].key == key) {
            return finite_key_lookup[i].name;
        }
    }
    return "Invalid";
}

void finite_keyboard_destroy_debug(const char *file, const char *func, int line, FiniteKeyboard *board) {
    wl_keyboard_destroy(board->keyboard);
    wl_seat_destroy(board->input->seat);
    wl_registry_destroy(board->input->registry);
    wl_display_disconnect(board->input->display);

    xkb_state_unref(board->xkb_state);
    xkb_keymap_unref(board->xkb_keymap);
    xkb_context_unref(board->xkb_ctx);

    free(board->input);
    free(board);

    FINITE_LOG("Keyboard cleaned.");
}

void finite_input_poll_keys_debug(const char *file, const char *func, int line, FiniteKeyboard *board, FiniteShell *shell) {
    // wl_display_dispatch_pending(board->input->display);
    // ? buck bumble

    // update buttons
    if (shell->btns != NULL) {
        FiniteBtn *self = shell->btns[shell->activeButton];
        if ( shell->_btns > 1 ) {
            if (finite_key_pressed(FINITE_KEY_DOWN, board)) {
                // go to the next based on if there are relations between buttons
                FINITE_LOG("Current btn: %d/%d", shell->activeButton, (shell->_btns - 1));
                if (self->relations && self->relations->down >= 0 ) {
                    self->isActive = false;
                    if (self->on_unfocus_callback) {
                        self->on_unfocus_callback(self, self->id, self->data);
                    }
                    int next = self->relations->down;
                    if (next < shell->_btns) {  
                        self = shell->btns[next];
                        if (!self) {
                            FINITE_LOG_WARN("No next available");
                            return;
                        }

                        FINITE_LOG("New button: %d", self->id);
                        self->isActive = true;
                        shell->activeButton = self->id;
                        if (self->on_focus_callback) {
                            self->on_focus_callback(self, self->id, self->data);
                        }
                        FINITE_LOG("New button: %d", shell->activeButton);
                    }

                } else {
                    int next = shell->activeButton + 1;
                    if (next < shell->_btns) {                        
                        self->isActive = false;
                        if (self->on_unfocus_callback) {
                            self->on_unfocus_callback(self, self->id, self->data);
                        }
                        self = shell->btns[next];
                        if (self != NULL) {
                            FINITE_LOG_ERROR("No next available");
                        } else {
                            self->isActive = true;
                            if (self->on_focus_callback) {
                                self->on_focus_callback(self, self->id, self->data);
                            }
                            shell->activeButton = next;
                        }
                    } else {
                        // no loop back
                        FINITE_LOG_WARN("No next available");
                    }
                }
            }

            if (finite_key_pressed(FINITE_KEY_UP, board)) {
                // go to the next based on if there are relations between buttons
                FINITE_LOG("Current btn: %d/%d", shell->activeButton, (shell->_btns - 1));

                if (self->relations && self->relations->up >= 0) {
                    self->isActive = false;
                    if (self->on_unfocus_callback) {
                        self->on_unfocus_callback(self, self->id, self->data);
                    }
                    int next = self->relations->up;
                    self = shell->btns[next];
                    if (self) {
                        self->isActive = true;
                        shell->activeButton = self->id;
                        if (self->on_focus_callback) {
                            self->on_focus_callback(self, self->id, self->data);
                        }
                    } else {
                        FINITE_LOG_WARN("No next available");
                        return;
                    }
                }
            }

            if (finite_key_pressed(FINITE_KEY_RIGHT, board)) {
                // go to the next based on if there are relations between buttons
                FINITE_LOG("Current btn: %d/%d", shell->activeButton, (shell->_btns - 1));

                if (self->relations && self->relations->right >= 0) {
                    self->isActive = false;
                    if (self->on_unfocus_callback) {
                        self->on_unfocus_callback(self, self->id, self->data);
                    }
                    int next = self->relations->right;
                    self = shell->btns[next];
                    if (self) {
                        self->isActive = true;
                        shell->activeButton = self->id;
                        if (self->on_focus_callback) {
                            self->on_focus_callback(self, self->id, self->data);
                        }
                    } else {
                        FINITE_LOG_WARN("No next available");
                        return;
                    }
                }
            }

            if (finite_key_pressed(FINITE_KEY_LEFT, board)) {
                // go to the next based on if there are relations between buttons
                FINITE_LOG("Current btn: %d/%d", shell->activeButton, (shell->_btns - 1));

                if (self->relations && self->relations->left >= 0) {
                    self->isActive = false;
                    if (self->on_unfocus_callback) {
                        self->on_unfocus_callback(self, self->id, self->data);
                    }
                    int next = self->relations->left;
                    self = shell->btns[next];
                    if (self) {
                        self->isActive = true;
                        shell->activeButton = self->id;
                        if (self->on_focus_callback) {
                            self->on_focus_callback(self, self->id, self->data);
                        }
                    } else {
                        FINITE_LOG_WARN("No next available");
                        return;
                    }
                }
            }
        }
        // handle selected
        if (finite_key_pressed(FINITE_KEY_ENTER, board)) {
            FINITE_LOG("Current btn: %d/%d", shell->activeButton, (shell->_btns - 1));

            if (self->on_select_callback) {
                self->on_select_callback(self, self->id, self->data);
            }
        }
    }

}
