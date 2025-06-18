#include "include/input/input-core.h"
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

/*
    # islands_registry_handle()

    This handles new connections to the registry so we can find the compositor
*/
void islands_key_registry_handle(void *data, struct wl_registry* registry, uint32_t id, const char* interface, uint32_t version) {
    FiniteInput *input = data;
    // we want to save the found compositor so that we can attach it to the finite_render_info event
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        input->isle = wl_registry_bind(registry, id, &wl_compositor_interface, 4);
        if (!input->isle) {
            printf("Something went wrong?\n");
        } else {
            printf("%p\n", input->isle);
        }
    }
    if (strcmp(interface, wl_seat_interface.name) == 0) {
        input->seat = wl_registry_bind(registry, id, &wl_seat_interface, 1);
    }
}   

// exists to satisfy the spec
void islands_key_registry_handle_remove(void *data, struct wl_registry* registry, uint32_t id) {
    return;
}

void islands_key_map_handle(void *data, struct wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size) {
    FiniteKeyboard *board = data;
    printf("[Finite] - Attempting to map\n");
    // verify the passed keyboard is the keyboard this event is handling
    if (keyboard != board->keyboard) {
        printf("[Finite] - Keyboard beind handled by islands_key_map_handle does not match the given FiniteKeyboard.");
        return;
    }

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        close(fd);
        return;
    }

    void *map_shm = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (map_shm == MAP_FAILED) {
        close(fd);
        return;
    }

    board->xkb_ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    board->xkb_keymap = xkb_keymap_new_from_string(board->xkb_ctx, map_shm, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_MAP_COMPILE_NO_FLAGS);
    munmap(map_shm, size);
    close(fd);

    if (!board->xkb_keymap) {
        printf("[Finite] - Attempted creating a valid xkb_keymap but was unable to.");
        return;
    }

    board->xkb_state = xkb_state_new(board->xkb_keymap);
}

void islands_key_handle(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    printf("[Finite] - Attempting to handle keys\n");
    // handle key input
    FiniteKeyboard *input = data;

    // verify the passed keyboard is the keyboard this event is handling
    if (keyboard != input->keyboard) {
        printf("[Finite] - Keyboard beind handled by islands_key_handle does not match the given FiniteKeyboard.");
        return;
    }

    if (key < 256) {
        input->keys[key].isDown = (state == WL_KEYBOARD_KEY_STATE_PRESSED);
        input->keys[key].isUp = (state == WL_KEYBOARD_KEY_STATE_RELEASED);
    } else {
        printf("[Finite] - Key %d is out of range for supported keys. Ignoring.", key);
    }
}

void islands_key_mod_handle(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
    printf("[Finite] - Attempting to handle modifiers. \n");
    // the only thing handled by default is if the meta key is pressed we want to call the suspend process and homescreen calls. For now let's just pass a message saying they've been called
    FiniteKeyboard *input = data;

    // verify the passed keyboard is the keyboard this event is handling
    if (keyboard != input->keyboard) {
        printf("[Finite] - Keyboard beind handled by islands_key_mod_handle does not match the given FiniteKeyboard.");
        return;
    }

    xkb_state_update_mask(input->xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);

    if (xkb_state_mod_name_is_active(input->xkb_state, XKB_MOD_NAME_LOGO, XKB_STATE_MODS_DEPRESSED)) {
        printf("Home key was pressed!\n");
    }
}

void islands_key_enter_handle(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
    // optional: log or store focus info
    printf("[Finite] - Keyboard focus received\n");
    (void)data; (void)keyboard; (void)serial; (void)surface; (void)keys;
}