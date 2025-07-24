#include "include/input/input.h"
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
    wl_display_dispatch_pending(board->input->display);

    // update buttons
    if (shell->btns != NULL) {
        FiniteBtn *self = shell->btns[shell->activeButton];
        if ( shell->_btns > 1 ) {
            if (finite_key_pressed(FINITE_KEY_DOWN, board)) {
                // go to the next based on if there are relations between buttons
                FINITE_LOG("[Finite] Current btn: %d/%d", shell->activeButton, shell->_btns);
                if (self->relations && self->relations->down >= 0) {
                    self->isActive = false;
                    self->on_unfocus_callback(self, self->id, self->data);
                    int next = self->relations->down;
                    self = shell->btns[next];
                    if (self) {
                        self->isActive = true;
                        self->on_focus_callback(self, self->id, self->data);
                        shell->activeButton = self->id;
                    } else {
                        FINITE_LOG_WARN("No next available");
                        return;
                    }
                } else {
                    int next = shell->activeButton + 1;
                    if (next >= shell->_btns) {
                        // no loop back
                        FINITE_LOG_WARN("No next available");
                    } else {
                        self->isActive = false;
                        self->on_unfocus_callback(self, self->id, self->data);
                        self = shell->btns[next];
                        self->isActive = true;
                        self->on_focus_callback(self, next, self->data);
                        shell->activeButton = next;
                    }
                }
            }

            if (finite_key_pressed(FINITE_KEY_UP, board)) {
                // go to the next based on if there are relations between buttons
                FINITE_LOG("[Finite] Current btn: %d/%d", shell->activeButton, shell->_btns);
                if (self->relations && self->relations->up >= 0) {
                    self->isActive = false;
                    self->on_unfocus_callback(self, self->id, self->data);
                    int next = self->relations->up;
                    self = shell->btns[next];
                    if (self) {
                        self->isActive = true;
                        self->on_focus_callback(self, self->id, self->data);
                        shell->activeButton = self->id;
                    } else {
                        FINITE_LOG_WARN("No next available");
                        return;
                    }
                }
            }

            if (finite_key_pressed(FINITE_KEY_RIGHT, board)) {
                // go to the next based on if there are relations between buttons
                FINITE_LOG("[Finite] Current btn: %d/%d", shell->activeButton, shell->_btns);
                if (self->relations && self->relations->right >= 0) {
                    self->isActive = false;
                    self->on_unfocus_callback(self, self->id, self->data);
                    int next = self->relations->right;
                    self = shell->btns[next];
                    if (self) {
                        self->isActive = true;
                        self->on_focus_callback(self, self->id, self->data);
                        shell->activeButton = self->id;
                    } else {
                        FINITE_LOG_WARN("No next available");
                        return;
                    }
                }
            }

            if (finite_key_pressed(FINITE_KEY_LEFT, board)) {
                // go to the next based on if there are relations between buttons
                FINITE_LOG("[Finite] Current btn: %d/%d", shell->activeButton, shell->_btns);
                if (self->relations && self->relations->left >= 0) {
                    self->isActive = false;
                    self->on_unfocus_callback(self, self->id, self->data);
                    int next = self->relations->left;
                    self = shell->btns[next];
                    if (self) {
                        self->isActive = true;
                        self->on_focus_callback(self, self->id, self->data);
                        shell->activeButton = self->id;
                    } else {
                        FINITE_LOG_WARN("No next available");
                        return;
                    }
                }
            }
        }
        // handle selected
        if (finite_key_pressed(FINITE_KEY_ENTER, board)) {
            FINITE_LOG("[Finite] Current btn: %d/%d", shell->activeButton, shell->_btns);
            self->on_select_callback(self, self->id, self->data);
        }
    }

}
