#include "include/input/input.h"
#include "include/input/keyboard.h"

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

static uint16_t finite_key_to_evdev(FiniteKey key) {
    size_t count = sizeof(finite_key_lookup) / sizeof(finite_key_lookup[0]);
    for (size_t i = 0; i < count; i++) {
        if (finite_key_lookup[i].key == key) {
            return finite_key_lookup[i].evdev_code;
        }
    }
    return UINT16_MAX; // Not found
}


bool finite_key_valid(FiniteKey key) {
    return finite_key_to_evdev(key) != UINT16_MAX;
}


// ALWAYS check the that the key is valid with finite_key_valid
bool finite_key_down(FiniteKey key, FiniteKeyboard *board) {
    if (!board) {
        printf("[Finite] - Unable to get input from NULL device");
        return false;
    }
    if (key == FINITE_KEY_INVALID) {
        printf("[Finite] - Unable to get input from Invalid key");
        return false;
    }

    uint16_t evdev_key = finite_key_to_evdev(key);
    if (evdev_key == UINT16_MAX) {
        printf("[Finite] - Unable to get input from Invalid key");
        return false;
    }

    return board->keys_down[evdev_key];
}

// ALWAYS check the that the key is valid with finite_key_valid
bool finite_key_up(FiniteKey key, FiniteKeyboard *board) {
    if (!board) {
        printf("[Finite] - Unable to get input from NULL device");
        return false;
    }
    if (key == FINITE_KEY_INVALID) {
        printf("[Finite] - Unable to get input from Invalid key");
        return false;
    }

    uint16_t evdev_key = finite_key_to_evdev(key);
    if (evdev_key == UINT16_MAX) {
        printf("[Finite] - Unable to get input from Invalid key");
        return false;
    }

    return board->keys_up[evdev_key];
}

FiniteKey finite_key_from_string(const char *name) {
    size_t count = sizeof(finite_key_lookup) / sizeof(finite_key_lookup[0]);
    for (size_t i = 0; i < count; i++) {
        if (strcasecmp(name, finite_key_lookup[i].name) == 0) {
            return finite_key_lookup[i].key;
        }
    }
    return FINITE_KEY_INVALID;
}


char *finite_key_string_from_key(FiniteKey key) {
    size_t count = sizeof(finite_key_lookup) / sizeof(finite_key_lookup[0]);
    for (size_t i = 0; i < count; i++) {
        if (finite_key_lookup[i].key == key) {
            return finite_key_lookup[i].name;
        }
    }
    return "Invalid";
}