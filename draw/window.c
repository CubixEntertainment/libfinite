#include "../include/draw/window.h"
#include "../include/log.h"

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
FiniteShell *finite_shell_init_debug(const char *file, const char *func, int line, char *device) {
    if (!device) {
        finite_log_internal(LOG_LEVEL_WARN, file, line, func, "Device %s is NULL or undefined. Defaulting to device \"wayland-0\"");
        device = "wayland-0";
    }

    FiniteShell *shell = calloc(1, sizeof(FiniteShell));
    FINITE_LOG("Device: %s", device); // debug stuff should come from this function
    shell->display = wl_display_connect(device);
    if (!shell->display) {
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to attach to the window"); // errors should be sent from the caller
    }

    shell->registry = wl_display_get_registry(shell->display);
    if (!shell->registry) {
        wl_display_disconnect(shell->display);
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to find valid registry with display %p", shell->display);
    }

    FINITE_LOG("Registry created.");
    FINITE_LOG("Adding listeners to registry.");
    wl_registry_add_listener(shell->registry, &registry_listener, shell); // pass the shell to store the globals
    wl_display_roundtrip(shell->display);
    FINITE_LOG("Compositor is at memory address %p", shell->isle);
    if (!shell->isle) {
        wl_display_disconnect(shell->display);
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to find a compositor");
    }

    if (!shell->base) {
        wl_display_disconnect(shell->display);
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to find a xdg_base");
    }

    if (!shell->output) {
        wl_display_disconnect(shell->display);
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to find a wl_output");
    }

    FINITE_LOG("Compositor with xdg found. Adding new surface");

    shell->isle_surface = wl_compositor_create_surface(shell->isle);
    if (!shell->isle_surface) {
        wl_display_disconnect(shell->display);
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to create a wl_surface with the given compsositor %p (Is it still running?", shell->isle);
        return NULL;
    }

    FINITE_LOG("Created a surface safely.");

    return shell;
}   

/*
    # finite_window_init

    Attempts to create an xdg_surface on top of a given shell. If making popups or overlays, you should use `finite_overlay_init`
*/
void finite_window_init_debug(const char *file, const char *func, int line, FiniteShell *shell) {
    if (!shell) {
        // if no shell throw an error
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to attach to a NULL shell.");
        shell = NULL;
        return;
    }
    FINITE_LOG("Attempting to make a window");
    shell->surface = xdg_wm_base_get_xdg_surface(shell->base, shell->isle_surface);
    if (!shell->surface) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to create a xdg_surface.");
        wl_display_disconnect(shell->display);
        shell = NULL;
        return;
    }

    xdg_surface_add_listener(shell->surface, &surface_listener, shell);

    FINITE_LOG("Attempting to get a toplevel");

    shell->window = xdg_surface_get_toplevel(shell->surface);
    if (!shell->window) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, " Unable to create a window.");
        wl_display_disconnect(shell->display);
        shell = NULL;
        return;
    }

    FINITE_LOG("Initialization Done.");

    xdg_toplevel_add_listener(shell->window, &toplevel_listener, shell);

    FINITE_LOG("Setting window size.");
    // grab screen size and store it
    wl_output_add_listener(shell->output, &output_listener, shell);

    wl_display_roundtrip(shell->display);
    if (!shell->details) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to create window geometry with NULL information.");
        wl_display_disconnect(shell->display);
        shell = NULL;
        return;
    }

    wl_surface_commit(shell->isle_surface);
    wl_display_roundtrip(shell->display);
    FINITE_LOG("Window made.");
}

/*
    # finite_overlay_init

    Attempts to create a layer_shell_surface on top of a given shell. If making windows (which you probably are), you should use `finite_window_init`
*/
void finite_overlay_init_debug(const char *file, const char *func, int line, FiniteShell *shell, int layer, char *name) {
    if (!shell) {
        // if no shell throw an error
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func,"Unable to attach to a NULL shell. "); // TODO create a finite_log function
        shell = NULL;
        return;
    }

    if (!shell->shell) {
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func,"Unable to access Layer Shell. (Does this environment support it?)");
    }

    if (!name) {
        name = "libfinite_layer";
    }
    shell->layer_surface = zwlr_layer_shell_v1_get_layer_surface(shell->shell, shell->isle_surface, shell->output, layer, name);    

    if (!shell->layer_surface) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to attach a NULL layer_surface a shell. "); 
        shell = NULL;
        return;
    }

    return;
}

/*
    # finite_window_size_set

    Attempts to set the bounding size of the window. This function should be called BEFORE attempting to draw anything to the screen with `finite_draw()` or `finite_shm_allocate()`.

    @note Unless you need a specific sized window, you don't need to call this as it is run silently when the output is added to the listener registry in `finite_shell_init()`.
*/
void finite_window_size_set_debug(const char *file, const char *func, int line, FiniteShell *shell, int xPos, int yPos, int width, int height) {
    FiniteWindowInfo *det = shell->details;

    if (!det) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to allocate memory for the window details");
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
    
    FINITE_LOG("Set window info to %d x %d at (%d,%d)", det->width, det->height, det->xPos, det->yPos);
}