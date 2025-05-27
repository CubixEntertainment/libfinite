#include "render/include/render.h"

const char* required_extensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
};

// keep track of the last compositor we found. Since Islands SHOULD be the only compositor running in prod, this makes lookup faster
struct wl_compositor *island;

/*
    islands_registry_handle()

    This handles new connections to the registry so we can find the compositor
*/
static void islands_registry_handle(void* data, struct wl_registry* registry, uint32_t id, const char* interface, uint32_t version) {
    // we want to save the found compositor so that we can attach it to the finite_render_info event
    if (strcmp(interface, "wl_compositor") == 0) {
        island = wl_registry_bind(registry, id, &wl_compositor_interface, 4);
    }
}

static void islands_registry_handle_remove(void* data, struct wl_registry* registry, uint32_t id);

// create a registry_listener struct for future use
static const struct wl_registry_listener registry_listener = {
    .global = islands_registry_handle,
    .global_remove = islands_registry_handle_remove
};

/*
    finite_render_create()

    Create a new renderer instance. Uses Vulkan by default.
    @param renderMode The rendering mode to use. If NULL it's set to `FIN_RENDERER_VULKAN`
    @param wayland_device The socket the current wayland session is running on. If NULL, wayland reads it as 'wayland-0'
*/
struct finite_render_info *finite_render_create(enum finite_render_type *renderMode, char *wayland_device, uint32_t version) {
    // by default, finite uses Vulkan. If renderMode is NULL, use vulkan.
    if (!renderMode) {
        renderMode = FIN_RENDERER_VULKAN;
    }

    // by default, finite uses 0.0.1 as the version. If version is NULL, use 0.0.1.
    if (!version) {
        version = VK_MAKE_API_VERSION(0,0,1);
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
    if (!island) {
        // TODO: Error out wito finite_log()
        wl_display_disconnect(render->display);
        free(render);
        free(regis);
        return NULL;
    }

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
        .pName = "libfinite Instance", // TODO: Allow devs to set the name
        .applicationVersion = VK_MAKE_API_VERSION(0,0,1), // TODO: Allow devs to define this themselves
        .pEngineName = "Engine", // TODO: Allow devs to set engine
        .engineVersion VK_MAKE_API_VERSION(0,0,1), // TODO: Allow devs to pass their engine version
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
    render->clientName = "libfinite instance";
    render->version = VK_MAKE_API_VERSION(0,0,1);
    render->engine = "AnyEngine";
    render->engineVersion = VK_MAKE_API_VERSION(0,1,1);
}

/*
    finite_render_remove()

    Cleans up the finite_render_info instance
    @param renderer The finite_render_info struct to cleanup
*/
void finite_render_remove(struct finite_render_info *render) {
    vkDestroySurfaceKHR(render->vk_instance, render->vk_surface, NULL);
    vkDestroyInstance(render->vk_instance, NULL);
    wl_display_disconnect(render->display);
    free(render);
}

/*
    finite_render_create_version

    Creates a valid Vulkan version number. Uses semantic versioning.
*/
uint32_t finite_render_create_version(int major, int minor, int patch) {
    return VK_MAKE_API_VERSION(major, minor, patch);
}