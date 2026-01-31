#include "../include/draw/window.h"
#include "../include/log.h"
#include "protocol/virtual-keyboard-client-protocol.h"
#include <unistd.h>


const struct wl_output_listener output_listener = {
    .geometry = islands_output_handle,
    .mode = islands_mode_handle,
    .scale = islands_scale_handle,
    .done = islands_done_handle
};


/*
    # islands_registry_handle()

    This handles new connections to the registry so we can find the compositor
*/
void islands_registry_handle(void *data, struct wl_registry* registry, uint32_t id, const char* interface, uint32_t version) {
    FiniteShell *shell = data;
    FINITE_LOG("Interface: %s", interface);
    // we want to save the found compositor so that we can attach it to the finite_render_info event
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        shell->isle = wl_registry_bind(registry, id, &wl_compositor_interface, 4);
        if (!shell->isle) {
            FINITE_LOG_ERROR("Unable to bind Wayland compositor %p to shell (%p)", shell->isle, shell);
        } else {
            FINITE_LOG("Found Wayland Compositor stored as %p", shell->isle);
        }
    }
    if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        shell->base = wl_registry_bind(registry, id, &xdg_wm_base_interface, 1);
    }
    if (strcmp(interface, wl_output_interface.name) == 0) {
        shell->output = wl_registry_bind(registry, id, &wl_output_interface, 2);
        FINITE_LOG("Setting window size.");
        // grab screen size and store earlier
        wl_output_add_listener(shell->output, &output_listener, shell);
    }
    if (strcmp(interface, wl_shm_interface.name) == 0) {
        shell->shm = wl_registry_bind(registry, id, &wl_shm_interface, 1);
    }
    if (strcmp(interface, wl_seat_interface.name) == 0) {
        FINITE_LOG("Added seat to shell");
        shell->seat = wl_registry_bind(registry, id, &wl_seat_interface, 1);
    }
    if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
        FINITE_LOG("Added layer-shell to shell");
        shell->shell = wl_registry_bind(registry, id, &zwlr_layer_shell_v1_interface, 4);
    }
    if (strcmp(interface, zwp_virtual_keyboard_manager_v1_interface.name) == 0) {
        FINITE_LOG("Added Virtual Keyboard to shell");
        shell->virtual_manager = wl_registry_bind(registry, id, &zwp_virtual_keyboard_manager_v1_interface, 1);
    }
}   

// exists to satisfy the spec
void islands_registry_handle_remove(void *data, struct wl_registry* registry, uint32_t id) {
    return;
}

/*
    # islands_output_handle()

    Handles getting output details for centering the window
*/  
void islands_output_handle(void *data, struct wl_output *output, int32_t x, int32_t y, int32_t physical_width, int32_t physical_height, int32_t subpixel, const char *make, const char *model, int32_t transform) {
    return;
}

void islands_mode_handle(void *data, struct wl_output *output, uint32_t flags, int32_t width, int32_t height, int32_t refresh) {
    FiniteShell *shell = data;
    FiniteWindowInfo *det = calloc(1, sizeof(FiniteWindowInfo));
    if (!det) {
        shell->details = NULL;
        FINITE_LOG_FATAL("Unable to allocate memory for the window details");
        return;
    }
    
    // We can modify details about the window later, for now give it the default values. 
    det->xPos = 0;
    det->yPos = 0;
    det->width = width;
    det->height = height;
    det->output = output;
    shell->details = det;
    
    FINITE_LOG("Set window info to %d x %d at (%d,%d)", det->width, det->height, det->xPos, det->yPos);
}

void islands_scale_handle(void *data, struct wl_output *wl_output, int32_t factor) {
    return;
}

void islands_done_handle(void *data, struct wl_output *wl_output) {
    return;
}

/*
    Handle XDG
*/
void window_configure_handle(void *data,struct xdg_toplevel *xdg_toplevel,int32_t width,int32_t height,struct wl_array *states) {
    FINITE_LOG("Desired sizing: %d x %d", width, height);
}

void window_close_handle(void *data, struct xdg_toplevel *xdg_toplevel) {
    FiniteShell *shell = data;
    FINITE_LOG_INFO("Close Requested.");
    cairo_surface_destroy(shell->cairo_surface);
    munmap(shell->pool_data, shell->pool_size);
    close(shell->shm_fd);
    wl_buffer_destroy(shell->buffer);
    wl_shm_pool_destroy(shell->pool);
    wl_shm_destroy(shell->shm);
    wl_display_disconnect(shell->display);
    free(shell->pool_data);
    free(shell->details);
    free(shell);
}

void window_bounds_handle(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height) {
    return;
}

void window_capable_handle(void *data, struct xdg_toplevel *xdg_toplevel, struct wl_array *capabilities) {
    return;
}

void surface_configure_handle(void *data, struct xdg_surface *xdg_surface, uint32_t serial) {
    FiniteShell *shell = data;
    FINITE_LOG("[Finite] - ack_configure request is sending");
    xdg_surface_ack_configure(xdg_surface, serial);
    FINITE_LOG("[Finite] - ack_configure request was sent.");

    FiniteWindowInfo *det = shell->details;

    xdg_surface_set_window_geometry(shell->surface, det->xPos, det->yPos, det->width, det->height);
}