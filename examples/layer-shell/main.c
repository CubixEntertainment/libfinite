#include "layer-shell-client-protocol.h" // from wayland scanner
#include <finite/draw.h>
#include <finite/log.h>

FiniteShell *shell;

int main() {
    finite_log_init(stdout, LOG_LEVEL_DEBUG, true);
    shell = finite_shell_init("wayland-0"); // get the device

    finite_overlay_init(shell, 3, "overlay");
    finite_overlay_set_size_and_position(shell, 200, 100, ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
    

    // try and draw
    // ! don't set the shm until AFTER calling finite_overlay_set_size_and_position
    finite_shm_alloc(shell, false);

    FiniteColorGroup test = { 0.827, 0.247, 0.286 };
    finite_draw_rect(shell, 0,0, 200, 100, &test, NULL);

    finite_draw_finish(shell, 200, 100, shell->stride, false);
    int state = wl_display_dispatch(shell->display);

    while (state != -1) {}
    FINITE_LOG("Done.");
	wl_surface_destroy(shell->isle_surface);
    finite_draw_cleanup(shell);
}