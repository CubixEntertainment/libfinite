/*
    Auth Request with Libfinite SDK example
    Written by Gabriel Thompson <gabriel.thomp@cubixdev.org>
*/
#include <finite/draw.h>
#include <finite/user.h>
#include <finite/log.h>

// boilerplate
FiniteShell *setup() {
    FiniteShell *myShell = finite_shell_init("wayland-0");
    
    if (!myShell) {
        FINITE_LOG("Unable to init shell");
        return NULL;
    }

    finite_window_init(myShell);

    if (!myShell) {
        FINITE_LOG("Unable to make shell");
        return NULL;
    }

    FiniteWindowInfo *det = myShell->details;
    int32_t width = det->width;
    int32_t height = det->height;

    finite_shm_alloc(myShell, true);

    FiniteColorGroup white = finite_draw_hex_to_color_group("#ffffff");
    FiniteColorGroup black = finite_draw_hex_to_color_group("#33333333");
    FiniteColorGroup text = finite_draw_hex_to_color_group("#ff4040");
    FiniteColorGroup btn = finite_draw_hex_to_color_group_alpha("#ca360075");

    finite_draw_rect(myShell, 0,0, width, height, &white, NULL);

    finite_draw_set_font(myShell, "Kumbh Sans", false, true, (height * 0.08));
    cairo_text_extents_t ext = finite_draw_get_text_extents(myShell, "Project Infinite");
    finite_draw_set_draw_position(myShell, ((width - ext.width) * 0.5), (height * 0.65));
    finite_draw_set_text(myShell, "Project Infinite", &text);

    finite_draw_set_font(myShell, "Kumbh Sans", false, false, (height * 0.02));
    ext = finite_draw_get_text_extents(myShell, "Introducing");
    finite_draw_set_draw_position(myShell, ((width - ext.width) * 0.5), (height * 0.5));
    finite_draw_set_text(myShell, "Introducing", &black);

    ext = finite_draw_get_text_extents(myShell, "Now loading...");
    finite_draw_set_draw_position(myShell, ((width - ext.width) * 0.5), (height * 0.85));
    finite_draw_set_text(myShell, "Now loading...", &btn);

    bool success = finite_draw_finish(myShell, width, height, myShell->stride, true);
    if (!success) {
        FINITE_LOG_ERROR("Something went wrong.\n");
        free(myShell);
    }

    return myShell;
}

int main() {
    finite_log_init(stdout, LOG_LEVEL_DEBUG, false);
    FiniteShell *shell = setup();
    // draw first frame
    wl_display_dispatch(shell->display);

    // sleep(3);

    // authenticate
    finite_user_request_auth(shell, "", "", NULL);
    
    while (wl_display_dispatch(shell->display) != -1) {

    }
    finite_draw_cleanup(shell);    
    wl_display_disconnect(shell->display);
}
