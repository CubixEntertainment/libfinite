#include "render/include/vulkan.h"


const char* required_extensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
};

// keep track of the last compositor we found. Since Islands SHOULD be the only compositor running in prod, this makes lookup faster
struct wl_compositor *island;
struct xdg_wm_base *base;

/*
    islands_registry_handle()

    This handles new connections to the registry so we can find the compositor
*/
static void islands_registry_handle(void* data, struct wl_registry* registry, uint32_t id, const char* interface, uint32_t version) {
    // we want to save the found compositor so that we can attach it to the finite_render_info event
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        island = wl_registry_bind(registry, id, &wl_compositor_interface, 4);
    }
    if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        base = wl_registry_bind(registry, id, &xdg_wm_base_interface, 1);
    }
}

static void islands_registry_handle_remove(void* data, struct wl_registry* registry, uint32_t id);

// create a registry_listener struct for future use
static const struct wl_registry_listener registry_listener = {
    .global = islands_registry_handle,
    .global_remove = islands_registry_handle_remove
};

static char *finite_render_get_engine_from_type(enum finite_render_engine engine) {
    switch (engine) {
        case FIN_RENDER_ENGINE_UNITY:
            return "Unity";
        case FIN_RENDER_ENGINE_GODOT:
            return "Godot";
        default:
            return "Unknown";
    }
}

/*
    finite_render_create_version

    Creates a valid Vulkan version number. Uses semantic versioning.
*/
uint32_t finite_render_create_version(int major, int minor, int patch) {
    return VK_MAKE_API_VERSION(0,major, minor, patch);
}

/*
    finite_render_info_create()

    Create a new renderer instance. Uses Vulkan by default.
    @param renderMode The rendering mode to use. If NULL it's set to `FIN_RENDERER_VULKAN`
    @param wayland_device The socket the current wayland session is running on. If NULL, wayland reads it as 'wayland-0'
    @param name The name of the app/game being made.
    @param version The version of the app/game being made. 
    @param engine The Engine being used to develop the game. Unless using a commercial engine, this can safely be NULL.
    @param engine_version The version of the engine being used. 
*/
struct finite_render_info *finite_render_info_create(enum finite_render_type *renderMode, char *wayland_device, char *name, uint32_t version, enum finite_render_engine engine, uint32_t engine_version) {
    // by default, finite uses Vulkan. If renderMode is NULL, use vulkan.
    if (!renderMode) {
        renderMode = FIN_RENDERER_VULKAN;
    }

    // if no wayland device is passed use the first available one (wayland-0)
    if (!wayland_device) {
        wayland_device = "wayland-0";
    }

    // If no name is passed, use a default name
    if (!name) {
        name = "Libfinite Window";
    }

    // by default, finite uses 0.0.1 as the version. If version is NULL, use 0.0.1.
    if (!version) {
        version = VK_MAKE_API_VERSION(0,0,0,1);
    }

    // If we can't determine the engine, use Unknown, which disables runtime optimizations (like Render Switching for Unity)
    if (!engine) {
        engine = FIN_RENDER_ENGINE_UNKNOWN;
    }

    // by default, finite uses 0.0.1 as the version. If version is NULL, use 0.0.1.
    if (!engine_version) {
        engine_version = VK_MAKE_API_VERSION(0,0,0,1);
    }

    // create the finite_render_info struct
    struct finite_render_info *render = calloc(1, sizeof(struct finite_render_info));
    // ensure that render is defined
    if (!render) {
        // TODO: Error out with finite_log(Finite_Error, msg)
        return NULL;
    }

    // connect to the display
    render->display = wl_display_connect(wayland_device);
    if (!render->display) {
        // TODO: Error out with finite_log()
        free(wayland_device);
        free(render);
        return NULL;
    }

    // now grab the surface
    struct wl_registry *regis = wl_display_get_registry(render->display);
    if (!regis) {
        // TODO: Error out with finite_log()
        free(render);
        return NULL;
    }    
    wl_registry_add_listener(regis, &registry_listener, NULL);
    // roundtrip to refresh information about the registry. This SHOULD grab the compositor
    wl_display_roundtrip(render->display);
    if (!island || !base) {
        // TODO: Error out with finite_log()
        wl_display_disconnect(render->display);
        free(render);
        free(regis);
        return NULL;
    }
    render->island = island;
    render->base = base;

    struct wl_surface* surface = wl_compositor_create_surface(island);
     if (!surface) {
        // TODO: Error out with finite_log()
        wl_display_disconnect(render->display);
        free(render);
        free(regis);
        return NULL;
    }
    render->surface = surface;

    // create a new Vulkan App information struct
    VkApplicationInfo base_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = NULL,
        .pApplicationName = name,
        .applicationVersion = version,
        .pEngineName = finite_render_get_engine_from_type(engine),
        .engineVersion = engine_version,
        .apiVersion = VK_API_VERSION_1_2
    };

    VkInstanceCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .pApplicationInfo = &base_info,
        .enabledExtensionCount = sizeof(required_extensions) / sizeof(required_extensions[0]),
        .ppEnabledExtensionNames = required_extensions,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL
    };

    // use info to create a new Vulkan Instance
    if (vkCreateInstance(&info, NULL, render->vk_instance) != VK_SUCCESS) {
        // TODO: Error out with finite_log()
        wl_display_disconnect(render->display);
        free(render);
        free(regis);
        return NULL;
    }

    VkWaylandSurfaceCreateInfoKHR island_info = {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .pNext = NULL,
        .display = render->display,
        .surface = render->surface
    };

    // use the islands_info to create a new vulkan surface
    if (vkCreateWaylandSurfaceKHR(render->display, &island_info, NULL, render->vk_surface) != VK_SUCCESS) {
        // TODO: Error out with finite_log()
        wl_display_disconnect(render->display);
        free(render);
        free(regis);
        return NULL;
    }

    // finally set some values and return
    // TODO: These should be from params
    render->clientName = name;
    render->version = version;
    render->engine = engine;
    render->engineVersion = engine_version;
}

/*
    finite_render_window_create()

    create a new window from the renderer.
    @param info The finite_render_info struct associated with the window.
*/
struct finite_render_window *finite_render_window_create(struct finite_render_info *info) {
    if (!info) {
        // TODO: Error out with finite_log()
        return NULL;
    }

    struct finite_render_window *window = calloc(1, sizeof( struct finite_render_window));
    // create the surface from the compositor
    window->wl_surface = wl_compositor_create_surface(info->island);
    window->surface = xdg_wm_base_get_xdg_surface(info->base, info->surface);
    window->toplevel = xdg_surface_get_toplevel(window->surface);

    // try to set fullscreen
    // ? If using Unity, double check the render option we're using
    if (info->engine != FIN_RENDER_ENGINE_UNITY) {
        xdg_toplevel_set_fullscreen(window->toplevel, NULL);
    } else {
        if (info->renderMode != FIN_RENDERER_UNITY) {
            xdg_toplevel_set_fullscreen(window->toplevel, NULL);
        }
    }

    // commit and roundtrip
    wl_surface_commit(window->wl_surface);
    wl_display_roundtrip(info->display);
    // vulkan here
    window->vk_surface = finite_render_vulkan_surface_create(info, window);

    window->info = info;
    return window;
}


/*
    finite_render_info_remove()

    Cleans up the finite_render_info instance
    @param renderer The finite_render_info struct to cleanup
*/
void finite_render_info_remove(struct finite_render_info *render) {
    vkDestroySurfaceKHR(render->vk_instance, render->vk_surface, NULL);
    vkDestroyInstance(render->vk_instance, NULL);
    wl_display_disconnect(render->display);
    free(render);
}

