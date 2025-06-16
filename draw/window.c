#include "../include/window.h"

// create a registry_listener struct for future use
const struct wl_registry_listener registry_listener = {
    .global = islands_registry_handle,
    .global_remove = islands_registry_handle_remove
};

const struct wl_output_listener output_listener = {
    .geometry = islands_output_handle,
    .mode = islands_mode_handle,
    .scale = islands_scale_handle,
    .done = islands_done_handle
};

const struct xdg_toplevel_listener toplevel_listener = {
    .configure = window_configure_handle,
    .close = window_close_handle,
    .configure_bounds = window_bounds_handle,
    .wm_capabilities = window_capable_handle
};

const struct xdg_surface_listener surface_listener = {
    .configure = surface_configure_handle
};

/*
    # finite_shell_init
    
    Returns an initialized FiniteShell. An initialized finite shell has all it wayland properties bound.
*/
FiniteShell finite_shell_init(char *device) {
    if (!device) {
        device = "wayland-0";
    }

    FiniteShell shell = {0};

    shell.display = wl_display_connect(device);
    if (!shell.display) {
        printf("[Finite] - Unable to attach to the window\n"); // TODO create a finite_log function
        return;
    }

    shell.registry = wl_display_get_registry(shell.display);
    if (!shell.registry) {
        printf("[Finite] - Unable to attach to the window\n"); // TODO create a finite_log function
        wl_display_disconnect(shell.display);
        return;
    }

    printf("[Finite] - Registry created.\n");
    printf("[Finite] - Adding listeners to registry.\n");
    wl_registry_add_listener(shell.registry, &registry_listener, &shell); // pass the shell to store the globals
    wl_display_roundtrip(shell.display);

    if (!shell.isle) {
        printf("[Finite] - Unable to find a compositor\n");
        wl_display_disconnect(shell.display);
        return;
    }

    if (!shell.base) {
        printf("[Finite] - Unable to find a xdg_wm_base\n");
        wl_display_disconnect(shell.display);
        return;
    }

    if (!shell.output) {
        printf("[Finite] - Unable to find a wl_output");
        wl_display_disconnect(shell.display);
        return;
    }

    printf("[Finite] - Compositor with xdg found. Adding new surface\n");

    shell.isle_surface = wl_compositor_create_surface(shell.isle);
    if (!shell.isle_surface) {
        printf("[Finite] - Unable to create a wl_surface with the given compsositor (Is it still running?)\n");
        wl_display_disconnect(shell.display);
        return;
    }

    printf("[Finite] - Created a surface safely.\n");

    return shell;
}   

/*
    # finite_window_init

    Attempts to create an xdg_surface on top of a given shell. If making popups or overlays, you should use `finite_overlay_init`
*/
void finite_window_init(FiniteShell *shell) {
    if (!shell) {
        // if no shell throw an error
        printf("[Finite] - Unable to attach to a NULL shell. \n"); // TODO create a finite_log function
        return;
    }

    shell->surface = xdg_wm_base_get_xdg_surface(shell->base, shell->isle_surface);
    if (!shell->surface) {
        printf("[Home] - Unable to create a xdg_surface.\n");
        wl_display_disconnect(shell->display);
        free(shell);
        return 1;
    }

    xdg_surface_add_listener(shell->surface, &surface_listener, shell);

    printf("[Home] - Attempting to get a toplevel \n");

    shell->window = xdg_surface_get_toplevel(shell->surface);
    if (!shell->window) {
        printf("[Home] - Unable to create a window.\n");
        wl_display_disconnect(shell->display);
        free(shell);
        return 1;
    }

    printf("[Home] - Initialization Done.\n");

    xdg_toplevel_add_listener(shell->window, &toplevel_listener, shell);

    printf("[Home] - Setting window size.\n");
    // grab screen size and store it
    wl_output_add_listener(shell->output, &output_listener, shell);

    wl_display_roundtrip(shell->display);
    if (!shell->details) {
        printf("[Home] - Unable to create window geometry with NULL information.\n");
        wl_display_disconnect(shell->display);
        free(shell);
        return 1;
    }

    wl_surface_commit(shell->isle_surface);
    wl_display_roundtrip(shell->display);
}

/*
    # finite_overlay_init

    Attempts to create a layer_shell_surface on top of a given shell. If making windows (which you probably are), you should use `finite_window_init`
*/
void finite_overlay_init(FiniteShell *shell, int layer, char *name) {
    if (!shell) {
        // if no shell throw an error
        printf("[Finite] - Unable to attach to a NULL shell. \n"); // TODO create a finite_log function
        return;
    }

    if (!shell->shell) {
        printf("[Finite] - Unable to access Layer Shell. (Does this environment support it?)");
        return;
    }

    if (!name) {
        name = "libfinite_layer";
    }
    shell->layer_surface = zwlr_layer_shell_v1_get_layer_surface(shell->shell, shell->isle_surface, shell->output, layer, name);    

    if (!shell->layer_surface) {
        printf("[Finite] - Unable to attach a NULL layer_surface a shell. \n"); 
        return;
    }

    return;
}

/*
    # finite_window_size_set

    Attempts to set the bounding size of the window. This function should be called BEFORE attempting to draw anything to the screen with `finite_draw()` or `finite_shm_allocate()`.

    @note Unless you need a specific sized window, you don't need to call this as it is run silently when the output is added to the listener registry in `finite_shell_init()`.
*/
void finite_window_size_set(FiniteShell *shell, int xPos, int yPos, int width, int height) {
    FiniteWindowInfo *det = shell->details;

    if (!det) {
        printf("[Home] - Unable to allocate memory for the window details");
        shell->details = NULL;
        return;
    }
    
    // We can modify details about the window later, for now give it the default values. 
    det->xPos = xPos;
    det->yPos = yPos;
    det->width = width;
    det->height = height;
    det->output = shell->output;
    shell->details = det;
    
    printf("[Home] - Set window info to %d x %d at (%d,%d)\n", det->width, det->height, det->xPos, det->yPos);
}