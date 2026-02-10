// handle text input
#include "cairo.h"
#include "draw/window.h"
#include "draw/cairo.h"
#include "include/log.h"
#include "input/textbox.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

static char *render_mode_to_string(FiniteTextboxRenderMode mode) {
    switch (mode) {
        case TEXTBOX_RENDER_MODE_EXPLICIT:
            return "explicit";
        case TEXTBOX_RENDER_MODE_IMPLICIT:
            return "implicit";
        case TEXTBOX_RENDER_MODE_BOTH:
            return "multi";
        default:
            return "unknown";
    }
}

static void send_signal(char *msg) {
    int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0);
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    char *path = getenv("FINITE_VK_DIR");
    if (!path) {
        FINITE_LOG_WARN("Unable to open the FINITE_VK_DIR socket.");
        close(fd);
        return;
    }

    strcpy(addr.sun_path, path);
    
    if (connect(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == 0) {
        write(fd, msg, strlen(msg));
    }

    close(fd);
}

FiniteTextbox *finite_input_textbox_create_debug(const char *file, const char *func, int line, FiniteShell *shell, FiniteTextboxDetails *details, FiniteTextboxRenderMode mode, void *data) {
    if (!shell) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Unable to attach to NULL shell");
        return NULL;
    }

    if (!shell->details) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Unable to attach to undefined or layer-shell window");
        return NULL;
    }

    FiniteTextbox *new_textbox = calloc(1, sizeof(FiniteTextbox));
    new_textbox->shell = shell;
    new_textbox->redraw_mode = mode;

    if (details == NULL && mode != TEXTBOX_RENDER_MODE_EXPLICIT) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Render mode is set to %s with no instructions on how to redraw. To handle redraw without libfinite set redraw mode to 'TEXTBOX_REDRAW_MODE_EXPLICIT'.", render_mode_to_string(mode));
        free(new_textbox);
        return NULL;
    }

    FINITE_LOG("Adding details");
    new_textbox->details = details; // even if details are null, reflect that
    
    new_textbox->data = data;

    return new_textbox;
}

void finite_input_textbox_set_callbacks_debug(const char *file, const char *func, int line, FiniteTextbox *textbox, void (*on_focused)(FiniteTextbox *self, FiniteTextboxCommitType type, void *data), void (*on_commit) (FiniteTextbox *self, FiniteTextboxCommitType type, void *data), void (*on_done)   (FiniteTextbox *self, FiniteTextboxCommitType type, void *data)) {
    if (!textbox) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Unable to set callbacks for NULL textbox");
        return;
    }

    if (textbox->redraw_mode == TEXTBOX_RENDER_MODE_IMPLICIT && on_commit != NULL) {
        finite_log_internal(LOG_LEVEL_WARN, file, line, func,  "The on_commit callback will never be called as redraw mode is set to %s. To recieve the callback, set redraw mode to 'TEXTBOX_REDRAW_MODE_BOTH`");
        return;
    }

    FINITE_LOG("Setting callbacks");

    if (TEXTBOX_RENDER_MODE_IMPLICIT) {
        textbox->on_commit = NULL;
    } else {
        textbox->on_commit = on_commit;
    }

    textbox->on_done = on_done;
    textbox->on_focused = on_focused;
}

void finite_input_textbox_request_input_debug(const char *file, const char *func, int line, FiniteTextbox *textbox, FiniteKeyboard *kbd) {
    if (!textbox) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Unable to request input for NULL textbox");
        return;
    }

    if (textbox->on_commit == NULL && textbox->redraw_mode != TEXTBOX_RENDER_MODE_IMPLICIT) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func,  "Render mode is set to %s with no instructions on how to commit. To let libfinite handle redraw set redraw mode to 'TEXTBOX_REDRAW_MODE_IMPLICIT'.", render_mode_to_string(textbox->redraw_mode));
        return;
    }

    if (textbox->on_done == NULL) {
        finite_log_internal(LOG_LEVEL_WARN, file, line, func,  "Render mode is set to %s with no instructions on how to handle finished events.", render_mode_to_string(textbox->redraw_mode));
    }

    if (textbox->on_focused == NULL) {
        finite_log_internal(LOG_LEVEL_WARN, file, line, func,  "Render mode is set to %s with no instructions on how to handle focus events.", render_mode_to_string(textbox->redraw_mode));
    }

    FINITE_LOG("Focusing textbox");
    if (!textbox->focused) {
        textbox->focused = true;
        if (textbox->on_focused) {
            textbox->on_focused(textbox, TEXTBOX_COMMIT_ENTER, textbox->data);
        }
    }

    FINITE_LOG("Validating keyboard");
    if (!kbd->keyboard) {
        // send socket signal for keyboard and assign the keyboard to be the new virtual keyboard
        send_signal("START\n");
        finite_input_keyboard_rescan_debug(file, func, line, kbd);
        if (!kbd->keyboard) {
            // if fail, finite_input_keyboard_rescan_debug will error on its own.
            return;
        }
    }

    FINITE_LOG("Validating focus");
    if (kbd->active_textbox != textbox) {
        // focus is removed with on_done. When keyboard focus is recieved, developers should 
        // make sure that it can not be removed until typing is finished.
        FINITE_LOG("Focusing keyboard.");
        kbd->active_textbox = textbox;
    }

}
