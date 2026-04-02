#include "cairo.h"
#include <finite/draw/cairo.h>
#define JSMN_HEADER

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "user.h"
#include "log.h"
#include "json.h"

static void send_signal(int fd, FiniteIPCRequest req) {
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    char *path = getenv("FINITE_IPC_MAIN_DIR");
    if (!path) {
        FINITE_LOG_WARN("Unable to open the FINITE_IPC_MAIN_DIR socket.");
        close(fd);
        return;
    }

    strcpy(addr.sun_path, path);
    
    if (connect(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == 0) {
        send(fd, &req, sizeof(FiniteIPCRequest), 0);
    } else {
        perror("connect");
    }
}

static FiniteShell *draw_auth_ui(FiniteWindowInfo *info, char *code) {
    FiniteShell *oshell = finite_shell_init("wayland-0");
    finite_overlay_init(oshell, 3, "authreq");

    double width = (double) info->width;
    double height = (double) info->height;
    double boxX = (width * 0.773);
    double boxY = (height * 0.714);

    if (!oshell) {
        // no auth overlay is a protocol 
        FINITE_LOG_FATAL("Unable to create overlay.");
        return NULL;
    } else {
        finite_overlay_set_size_and_position(oshell, width, height, ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);

        finite_shm_alloc(oshell, true);

        // draw base
        FiniteColorGroup white = finite_draw_hex_to_color_group("#ffffff");
        FiniteColorGroup black = finite_draw_hex_to_color_group("#1d1d1d");
        FiniteColorGroup gold = finite_draw_hex_to_color_group("#ffb957");
        FiniteColorGroup orange = finite_draw_hex_to_color_group("#ff7e0d");

        FiniteGradientPoint background[2] =  {
            {
                0, white.r, white.g, white.b, 0.8
            },
            {
                1, 0, 0, 0, 0.8
            }
        };

        cairo_pattern_t *pat = finite_draw_pattern_linear(0, 0, 0, height, background, 2);
        finite_draw_rect(oshell, 0, 0, width, height, NULL, pat);

        finite_draw_rounded_rect(oshell, (width * 0.113), (height * 0.152), boxX, boxY, (height * 0.06), &black, NULL, true);
        finite_draw_stroke(oshell, &orange, NULL, (height * 0.008));

        // clip
        finite_draw_rounded_rect(oshell, (width * 0.113), (height * 0.152), boxX, boxY, (height * 0.06), NULL, NULL, true);
        cairo_save(oshell->cr);
        cairo_clip(oshell->cr);
        cairo_translate(oshell->cr, (width * 0.113), (height * 0.152));


        finite_draw_set_font(oshell, "Kumbh Sans", false, true, (height * 0.04));
        finite_draw_set_draw_position(oshell, (boxX * 0.05), (boxY * 0.12));
        finite_draw_set_text(oshell, "This app wants you to authenticate", &white);
        cairo_text_extents_t ext = finite_draw_get_text_extents(oshell, "This app wants you to authenticate");

        finite_draw_set_font(oshell, "Kumbh Sans", false, false, (height * 0.02));
        finite_draw_set_draw_position(oshell, (boxX * 0.05), ((boxY - ext.height) * 0.22));
        finite_draw_set_text(oshell, "In order to log in, scan the QR code or visit", &white);
        ext = finite_draw_get_text_extents(oshell, "In order to log in, scan the QR code or visit");

        finite_draw_set_font(oshell, "Kumbh Sans", false, true, (height * 0.03));
        finite_draw_set_draw_position(oshell, (boxX * 0.05), ((boxY - ext.height) * 0.27));
        finite_draw_set_text(oshell, "https://activate.cubixdev.org", &gold);
        ext = finite_draw_get_text_extents(oshell, "https://activate.cubixdev.org");


        finite_draw_set_font(oshell, "Kumbh Sans", false, false, (height * 0.02));
        finite_draw_set_draw_position(oshell, (boxX * 0.05), ((boxY - ext.height) * 0.34));
        finite_draw_set_text(oshell, "and log into your Cubix account, then enter the code below.", &white);
        ext = finite_draw_get_text_extents(oshell, "and log into your Cubix account, then enter the code below.");


        finite_draw_set_font(oshell, "Kumbh Sans", false, false, (height * 0.032));
        finite_draw_set_draw_position(oshell, (boxX * 0.05), ((boxY - ext.height) * 0.43));
        finite_draw_set_text(oshell, "This prompt will disappear when you're done.", &white);
        ext = finite_draw_get_text_extents(oshell, "This prompt will disappear when you're done.");

        finite_draw_set_font(oshell, "Kumbh Sans", false, true, (height * 0.08));
        finite_draw_set_draw_position(oshell, (boxX * 0.05), ((boxY + ext.height) * 0.55));
        finite_draw_set_text(oshell, code, &white);

        // TODO: Get QR Code
        cairo_restore(oshell->cr);

        finite_draw_finish(oshell, width, height, oshell->stride, true);

        // hand control to shell to the caller
        return oshell;
    }
}

static enum FiniteAuthState to_auth_state(char *state) {
    if (strcmp(state, "Success") == 0) {
        return AUTH_STATE_SUCCESS;
    }
    if (strcmp(state, "Expired") == 0) {
        return AUTH_STATE_CODE_EXPIRED;
    }

    if (strcmp(state, "Error") == 0) {
        return AUTH_STATE_ERROR;
    }

    return AUTH_STATE_PENDING;
}

char *finite_auth_state_tostring(enum FiniteAuthState state) {
    switch (state) {
        case AUTH_STATE_SUCCESS:
            return "Success";
        case AUTH_STATE_CODE_EXPIRED:
            return "Code expired";
        case AUTH_STATE_ERROR:
            return "An error occured";
        default:
            return "Pending";
    }
}

/* 
    * Spec notes:
    * When the developer recieved the callback, they are to redraw pipeline stays open.
    * Game not using cairo rendering should handle overlays correctly.
    * Developers should NOT render their own auth interface. libfinite does that internally.
    * Developers are also responsible for re-rendering the screen once this returns.
*/

FiniteAuthRequest *finite_user_request_auth_debug(const char *file, const char *func, int line, FiniteShell *shell, char id[4], char token[64], void (*callback)(char *code)) {
    int fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (id == NULL) {
        id = "26";
    }



    FiniteIPCRequest request = {
        .cmd = "AUTH",
        .adr = "https://api.cubixdev.org", // when we send the AUTH command, adr is ignored
        .data = "", // auth ignores this 
        .device_id = "05b47eea-0149-4daf-8b3a-efe7532cdecb", // retreived from the device in prod
        .token = "", // retrived from the device in prod
        .user_id = "26", // auth treats user_id as game_id as of right now
        .write_mode = 0,
        .data_type = 0,
    };

    send_signal(fd, request);
    // the first message is hello message which we can safely ignore as long as error is false
    FiniteIPCResponse response;
    ssize_t n = recv(fd, &response, sizeof(FiniteIPCResponse), 0);
    if (n == 0) {
        finite_log_internal(LOG_LEVEL_DEBUG, file, line, func, "Buffer is 0. (Failed on handshake).");
        close(fd);
        return NULL;
    } else if (n < 0) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Something went wrong.");
        perror("recv");
        close(fd);
        return NULL;
    } else {
        if (response.status != 100) {
            finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Something went wrong.");
            finite_log_internal(LOG_LEVEL_INFO, file, line, func, "Info:\n\tStatus: %d\n\tData: %s", response.status, response.data);
            close(fd);
            return NULL;
        }
    }

    FiniteAuthRequest *code_data = calloc(1, sizeof(FiniteAuthRequest));

    // assuming we can auth, read the next message
    ssize_t coden = recv(fd, &response, sizeof(FiniteIPCResponse), 0);
    if (coden == 0) {
        finite_log_internal(LOG_LEVEL_DEBUG, file, line, func, "Buffer is 0. (Failed on code_get)");
        code_data->auth_state = AUTH_STATE_ERROR;
        code_data->state = "Internal Server Error.";
        close(fd);
        return code_data;
    } else if (n < 0) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Something went wrong during code_get.");
        code_data->auth_state = AUTH_STATE_ERROR;
        code_data->state = "Internal Server Error.";
        perror("recv");
        close(fd);
        return code_data;
    } else {
        // if error exit, otherwise request render
        if (response.status != 200) {
            finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Something went wrong during code_get.");
            finite_log_internal(LOG_LEVEL_INFO, file, line, func, "Info:\n\tStatus: %d\n\tData: %s", response.status, response.data);
            code_data->auth_state = AUTH_STATE_ERROR;
            code_data->state = response.data;
            close(fd);
            return code_data;
        }
    }

    // parse json
    FiniteJSONValue *json = finite_json_parse(response.data);
    if (!json) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Something went wrong while parsing data from code_get.");
        finite_log_internal(LOG_LEVEL_INFO, file, line, func, "Info:\n\tStatus: %d\n\tData: %s", response.status, response.data);
        code_data->auth_state = AUTH_STATE_ERROR;
        code_data->state = response.data;
        close(fd);
        return code_data;
    }

    char *code = finite_json_get_value_from_key(json, "verify_code");
    if (!code) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Item verify_code was not found.");
    }

    finite_log_internal(LOG_LEVEL_DEBUG, file, line, func, "Code: %s", code);

    // handle drawing here
    FiniteShell *oshell = draw_auth_ui(shell->details, code);
    
    ssize_t finn = recv(fd, &response, sizeof(FiniteIPCResponse), 0);
    if (finn == 0) {
        finite_log_internal(LOG_LEVEL_DEBUG, file, line, func, "Buffer is 0. (Failed on code_check)");
        code_data->auth_state = AUTH_STATE_ERROR;
        code_data->state = "Internal Server Error.";
        close(fd);
        return code_data;
    } else if (n < 0) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Something went wrong during code_check.");
        code_data->auth_state = AUTH_STATE_ERROR;
        code_data->state = "Internal Server Error.";
        perror("recv");
        close(fd);
        return code_data;
    } else {
        finite_draw_cleanup(oshell);
        // if error exit, otherwise request render
        if (response.status != 200) {
            finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Something went wrong during code_check.");
            finite_log_internal(LOG_LEVEL_INFO, file, line, func, "Info:\n\tStatus: %d\n\tData: %s", response.status, response.data);
            code_data->auth_state = AUTH_STATE_ERROR;
            code_data->state = response.data;
            close(fd);
            return code_data;
        }
    }
    
    finite_log_internal(LOG_LEVEL_DEBUG, file, line, func, "Successfully recieved auth state");
    finite_draw_cleanup(oshell);

    FiniteJSONValue *auth_info = finite_json_parse(response.data);
    if (!auth_info) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Something went wrong while parsing data from code_get.");
        finite_log_internal(LOG_LEVEL_INFO, file, line, func, "Info:\n\tStatus: %d\n\tData: %s", response.status, response.data);
        code_data->auth_state = AUTH_STATE_ERROR;
        code_data->state = response.data;
        close(fd);
        return code_data;
    }

    code_data->id = atoi(finite_json_get_value_from_key(json, "id"));
    code_data->iat = atoi(finite_json_get_value_from_key(json, "expires_at"));
    code_data->game_id = atoi(finite_json_get_value_from_key(json, "game_id"));
    code_data->state = finite_json_get_value_from_key(json, "state");
    // convert state
    code_data->auth_state = to_auth_state(code_data->state);
    
    code_data->device_id = finite_json_get_value_from_key(json, "device_id");
    code_data->verify_code = finite_json_get_value_from_key(json, "verify_code");
    code_data->user_id = finite_json_get_value_from_key(json, "user_id");

    return code_data;
}