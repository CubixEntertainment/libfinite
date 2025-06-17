#include "include/input/input.h"

// create a registry_listener struct for future use
const struct wl_registry_listener registry_listener = {
    .global = islands_registry_handle,
    .global_remove = islands_registry_handle_remove
};

const struct wl_keyboard_listener keys_listener = {
    .key = islands_key_handle,
    .modifiers = islands_key_mod_handle,
    .keymap = islands_key_map_handle,
    .enter = NULL,
    .leave = NULL,
    .repeat_info = NULL
};

FiniteKeyboard *finite_input_keyboard_init(char *device) {
    FiniteInput *input = calloc(1, sizeof(FiniteInput));

    input->display = wl_display_connect(device);
    if (!input->display) {
        printf("[Finite] - Unable to attach to display");
        free(input);
        return NULL;
    }

    input->registry = wl_display_get_registry(input->display);
    if (!input->registry) {
        printf("[Finite] - Unable to attach to registry");
        free(input);
        return NULL;
    }

    wl_registry_add_listener(input->registry, &registry_listener, input);
    wl_display_roundtrip(input->display);

    if (!input->isle) {
        printf("[Finite] - Unable to find a compositor.");
        free(input);
        return NULL;
    }

    if (!input->seat) {
        printf("[Finite] - Unable to find a wl_seat.");
        free(input);
        return NULL;
    }

    FiniteKeyboard *keyboard = calloc(1, sizeof(FiniteKeyboard));
    keyboard->input = input;
    // free the input we allocated and make a reference to the new one
    free(input);
    FiniteInput *input = keyboard->input;

    keyboard->keyboard = wl_seat_get_keyboard(keyboard->input->seat);
    if (!keyboard->keyboard) {
        printf("[Finite] - Unable to find a wl_seat.");
        free(input);
        free(keyboard);
        return NULL;
    }

    wl_keyboard_add_listener(keyboard->keyboard, &keys_listener, keyboard);
    return keyboard;
}

bool finite_input_keyboard_key_down();