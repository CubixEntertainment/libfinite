#include "include/input/input-core.h"
#include "../include/log.h"
#include "input/keyboard.h"
#include "input/textbox.h"
#include <ctype.h>
#include <linux/input-event-codes.h>
#include <wayland-client-protocol.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

const char *shift_to_char[10] = {
    ")",
    "!",
    "@",
    "#",
    "$",
    "%",
    "^",
    "&",
    "*",
    "("
};

const FiniteKeyMapping finite_key_local_lookup_table[] = {
    // Letters
    { "a", FINITE_KEY_A, KEY_A }, { "b", FINITE_KEY_B, KEY_B }, { "c", FINITE_KEY_C, KEY_C },
    { "d", FINITE_KEY_D, KEY_D }, { "e", FINITE_KEY_E, KEY_E }, { "f", FINITE_KEY_F, KEY_F },
    { "g", FINITE_KEY_G, KEY_G }, { "h", FINITE_KEY_H, KEY_H }, { "i", FINITE_KEY_I, KEY_I },
    { "j", FINITE_KEY_J, KEY_J }, { "k", FINITE_KEY_K, KEY_K }, { "l", FINITE_KEY_L, KEY_L },
    { "m", FINITE_KEY_M, KEY_M }, { "n", FINITE_KEY_N, KEY_N }, { "o", FINITE_KEY_O, KEY_O },
    { "p", FINITE_KEY_P, KEY_P }, { "q", FINITE_KEY_Q, KEY_Q }, { "r", FINITE_KEY_R, KEY_R },
    { "s", FINITE_KEY_S, KEY_S }, { "t", FINITE_KEY_T, KEY_T }, { "u", FINITE_KEY_U, KEY_U },
    { "v", FINITE_KEY_V, KEY_V }, { "w", FINITE_KEY_W, KEY_W }, { "x", FINITE_KEY_X, KEY_X },
    { "y", FINITE_KEY_Y, KEY_Y }, { "z", FINITE_KEY_Z, KEY_Z },

    // Digits
    { "0", FINITE_KEY_0, KEY_0 }, { "1", FINITE_KEY_1, KEY_1 }, { "2", FINITE_KEY_2,KEY_2 },
    { "3", FINITE_KEY_3, KEY_3 }, { "4", FINITE_KEY_4, KEY_4 }, { "5", FINITE_KEY_5, KEY_5 },
    { "6", FINITE_KEY_6, KEY_6 }, { "7", FINITE_KEY_7, KEY_7 }, { "8", FINITE_KEY_8, KEY_8 },
    { "9", FINITE_KEY_9, KEY_9 },

    // Modifiers
    { "#+=",  FINITE_KEY_LEFT_SHIFT, KEY_LEFTSHIFT },   
    { "Done",   FINITE_KEY_ESCAPE, KEY_ESC },  

    // Editing / navigation
    { "Return",      FINITE_KEY_ENTER, KEY_ENTER },
    { "Backspace",  FINITE_KEY_BACKSPACE, KEY_BACKSPACE },
    { " ",      FINITE_KEY_SPACE, KEY_SPACE },

    // Symbols and punctuation
    { "-",            FINITE_KEY_MINUS, KEY_MINUS },
    { "=",       FINITE_KEY_EQUALS, KEY_EQUAL },
    { "[",  FINITE_KEY_LEFT_BRACKET, KEY_LEFTBRACE },
    { "]", FINITE_KEY_RIGHT_BRACKET, KEY_RIGHTBRACE },
    { "/",    FINITE_KEY_BACKSLASH, KEY_BACKSLASH },
    { ";",            FINITE_KEY_SEMICOLON, KEY_SEMICOLON },
    { "'",         FINITE_KEY_APOSTROPHE, KEY_APOSTROPHE },
    { "`",        FINITE_KEY_GRAVE, KEY_GRAVE },
    { ",",        FINITE_KEY_COMMA, KEY_COMMA },
    { ".",       FINITE_KEY_PERIOD, KEY_DOT },
    { "/",           FINITE_KEY_SLASH, KEY_SLASH },
};

void redraw_implicit_commit(FiniteTextbox *textbox, FiniteTextboxCommitType commit, void *data) {
    if (!textbox) {
        FINITE_LOG_ERROR("Unale to redraw without textbox");
        return;
    }

    FiniteShell *shell = textbox->shell;
    FiniteTextboxDetails *details = textbox->details;
    if (!shell) {
        FINITE_LOG_ERROR("Unale to redraw to NULL shell");
        return;
    }

    if (!shell->cr) {
        shell->cr = cairo_create(shell->cairo_surface);
    }

    cairo_save(shell->cr);

    if (details->isRounded) {
        finite_draw_rounded_rect(shell, details->x - (details->stroke_weight), details->y - (details->stroke_weight), (details->width + details->stroke_weight), details->height + details->stroke_weight, details->box_radius, NULL, NULL, false);
    } else {
        finite_draw_rect(shell, details->x - (details->stroke_weight), details->y - (details->stroke_weight), (details->width + details->stroke_weight), details->height + details->stroke_weight, NULL, NULL);
    }

    cairo_clip(shell->cr);
    cairo_set_operator (shell->cr, CAIRO_OPERATOR_SOURCE);

    if (details->isRounded) {
        finite_draw_rounded_rect(shell, details->x, details->y, details->width, details->height, details->box_radius, &details->background_color, NULL, false);
    } else {
        finite_draw_rect(shell, details->x, details->y, details->width, details->height, &details->background_color, NULL);
    }

    cairo_restore(shell->cr);

    // draw the box now
    finite_draw_rounded_rect(shell, details->x, details->y, (details->width - ((double) details->stroke_weight / 2)) , (details->height - ((double) details->stroke_weight / 2)), details->box_radius, &details->box_color, NULL, details->stroke_weight > 0 ? true : false);
    if (details->stroke_weight > 0) {
        finite_draw_stroke(shell, &details->stroke_color, NULL, details->stroke_weight);
    }

    // draw the text within the box
    finite_draw_set_font(shell, details->text_font, false, false, details->text_size);
    cairo_text_extents_t ext = finite_draw_get_text_extents(shell, textbox->text);

    if (ext.width <= details->width) {
        switch (details->text_align) {
            case TEXTBOX_ALIGN_LEFT:
                finite_draw_set_draw_position(shell, details->x, (details->y + (details->height / 2) + (ext.height / 2)));
                break;
            case TEXTBOX_ALIGN_CENTER:
                finite_draw_set_draw_position(shell, (details->x + (details->width / 2)) - (ext.width / 2), details->y + ((details->height / 2) + (ext.height / 2)));
                break;
        }

        finite_draw_set_text(shell, textbox->text, &details->text_color);
        free(textbox->final_text);
        textbox->final_text = strdup(textbox->text);
    } else {
        // TODO: Allow devs to decide how to handle overflow
        ext = finite_draw_get_text_extents(shell, textbox->final_text);

        switch (details->text_align) {
            case TEXTBOX_ALIGN_LEFT:
                finite_draw_set_draw_position(shell, details->x, (details->y + (details->height / 2) + (ext.height / 2)));
                break;
            case TEXTBOX_ALIGN_CENTER:
                finite_draw_set_draw_position(shell, (details->x + (details->width / 2)) - (ext.width / 2), details->y + ((details->height / 2) + (ext.height / 2)));
                break;
        }


        finite_draw_set_text(shell, textbox->final_text, &details->text_color);  
    }

    if (textbox->redraw_mode == TEXTBOX_RENDER_MODE_BOTH) {
        //Dont redraw
        FINITE_LOG_INFO("Commit ready");
        textbox->on_commit(textbox, commit, data);
    } else {
        bool success = finite_draw_finish(shell, shell->details->width, shell->details->height, shell->stride, true);        
        if (!success) {
            FINITE_LOG_ERROR("Something is wrong.");
        }
    }
}

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
            FINITE_LOG_ERROR("Unable to find the assoicated compositor.");
        } else {
            FINITE_LOG("Found usable compositor at address %p", input->isle);
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
    FINITE_LOG("Attempting to map");
    // verify the passed keyboard is the keyboard this event is handling
    if (keyboard != board->keyboard) {
        FINITE_LOG_ERROR("Keyboard beind handled by islands_key_map_handle does not match the given FiniteKeyboard.");
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
        FINITE_LOG_ERROR("Attempted creating a valid xkb_keymap but was unable to.");
        return;
    }

    board->xkb_state = xkb_state_new(board->xkb_keymap);
}

void islands_key_handle(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    FINITE_LOG("Attempting to handle keys");
    // handle key input
    FiniteKeyboard *input = data;

    // verify the passed keyboard is the keyboard this event is handling
    if (keyboard != input->keyboard) {
        FINITE_LOG_ERROR("Keyboard being handled by islands_key_handle does not match the given FiniteKeyboard.");
        return;
    }

    if (key < 256) {
        // textboxes take priority from regular input
        if (input->active_textbox != NULL && state == WL_KEYBOARD_KEY_STATE_PRESSED) {
            FINITE_LOG("Textbox requested input");
            FiniteTextbox *textbox = input->active_textbox;
            FiniteKeyMapping map;
            bool mapFound = false;
            size_t count = sizeof(finite_key_local_lookup_table) / sizeof(finite_key_local_lookup_table[0]);
            for (size_t i = 0; i < count; i++) {
                if (finite_key_local_lookup_table[i].evdev_code == key) {
                    map = finite_key_local_lookup_table[i];
                    mapFound = true;
                }
            }

            if (mapFound) {
                FINITE_LOG("Current Key %s", map.name);
                if (map.key == FINITE_KEY_ESCAPE || map.key == FINITE_KEY_ENTER) {
                    // close event recieved
                    FINITE_LOG("Textbox requested close");
                    input->active_textbox = NULL;
                    if (textbox->on_done) {
                        textbox->on_done(textbox, TEXTBOX_COMMIT_FINISH, textbox->data);
                    }
                } else if (map.key == FINITE_KEY_LEFT_SHIFT || map.key == FINITE_KEY_RIGHT_SHIFT) {
                    FINITE_LOG("Shift key pressed");
                    input->isShift = true;
                } else if (map.key == FINITE_KEY_BACKSPACE) {
                    FINITE_LOG("Backspace recognized.");
                    size_t len = strlen(textbox->text);

                    if (len > 0) {
                        textbox->text[len - 1] = '\0';
                    }

                    if (textbox->redraw_mode != TEXTBOX_RENDER_MODE_EXPLICIT) {
                        redraw_implicit_commit(textbox, TEXTBOX_COMMIT_REMOVE, textbox->data);
                    }

                    if (textbox->redraw_mode != TEXTBOX_RENDER_MODE_IMPLICIT) {
                        textbox->on_commit(textbox, TEXTBOX_COMMIT_REMOVE, textbox->data);
                    }

                } else {
                    FINITE_LOG("Key press");
                    char *new_text = strdup(map.name);
                    if (input->isShift) {
                        if (finite_key_is_number(map.key)) {
                            const char *realStr = shift_to_char[atoi(map.name)];
                            memcpy(new_text, realStr, strlen(realStr));
                        } else {
                            new_text[0] = toupper(new_text[0]);
                        }
                    }
                    size_t len = textbox->text == NULL ? 0 : strlen(textbox->text);
                    char *text = malloc(len + strlen(new_text) + 1);
                    if (!text) {
                        FINITE_LOG_ERROR("Somehing went wrong while trying to realloc string");
                    }
                    if (len > 0) {
                        memcpy(text, textbox->text, len);
                    }

                    memcpy(text + len, new_text, strlen(new_text) + 1);
                    free(textbox->text);
                    textbox->text = text;

                    if (textbox->redraw_mode != TEXTBOX_RENDER_MODE_EXPLICIT) {
                        FINITE_LOG("Pushing internal commit");
                        redraw_implicit_commit(textbox, TEXTBOX_COMMIT_ADD, textbox->data);
                    }

                    if (textbox->redraw_mode != TEXTBOX_RENDER_MODE_IMPLICIT) {
                        FINITE_LOG("Pushing external commit");
                        textbox->on_commit(textbox, TEXTBOX_COMMIT_ADD, textbox->data);
                    }
                }

            }
        } else if (input->active_textbox != NULL && state == WL_KEYBOARD_KEY_STATE_RELEASED) {
            FiniteKeyMapping map;
            bool mapFound = false;
            size_t count = sizeof(finite_key_local_lookup_table) / sizeof(finite_key_local_lookup_table[0]);
            for (size_t i = 0; i < count; i++) {
                if (finite_key_local_lookup_table[i].evdev_code == key) {
                    map = finite_key_local_lookup_table[i];
                    mapFound = true;
                }
            }

            if (mapFound && (map.key == FINITE_KEY_LEFT_SHIFT || map.key == FINITE_KEY_RIGHT_SHIFT)) {
                FINITE_LOG("Shift key released");
                input->isShift = false;
            }
        } else {
            input->keys[key].isDown = (state == WL_KEYBOARD_KEY_STATE_PRESSED);
            input->keys[key].isUp = (state == WL_KEYBOARD_KEY_STATE_RELEASED);
        }

        
    } else {
        FINITE_LOG_ERROR("Key %d is out of range for supported keys. Ignoring.", key);
    }
}

void islands_key_mod_handle(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
    FINITE_LOG("Attempting to handle modifiers. ");
    // the only thing handled by default is if the meta key is pressed we want to call the suspend process and homescreen calls. For now let's just pass a message saying they've been called
    FiniteKeyboard *input = data;

    // verify the passed keyboard is the keyboard this event is handling
    if (keyboard != input->keyboard) {
        FINITE_LOG_ERROR("Keyboard beind handled by islands_key_mod_handle does not match the given FiniteKeyboard.");
        return;
    }

    xkb_state_update_mask(input->xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);

    if (xkb_state_mod_name_is_active(input->xkb_state, XKB_MOD_NAME_LOGO, XKB_STATE_MODS_DEPRESSED)) {
        FINITE_LOG_INFO("Home key was pressed!");
    }
}

void islands_key_enter_handle(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
    // optional: log or store focus info
    FINITE_LOG_INFO("Keyboard focus received");
    (void)data; (void)keyboard; (void)serial; (void)surface; (void)keys;
}

void islands_key_leave_handler(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface) {
    FINITE_LOG_INFO("Keyboard focus left surface");
}