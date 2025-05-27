#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define VK_USE_PLATFORM_WAYLAND_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>
#include <wayland-client.h>
#include <cglm/cglm.h>


enum finite_render_type {
    FIN_RENDERER_UNITY,
    FIN_RENDERER_VULKAN,
    FIN_RENDERER_OPENGL
};

/*
    Basic information about the renderer.
*/
struct finite_render_info {
    struct wl_display *display;
    struct wl_surface *surface;
    struct wl_compositor *island; // Islands is the Infinite compositor
    uint32_t version;
    uint16_t engineVersion;
    char clientName;
    char engine;
    VkInstance vk_instance;
    VkSurfaceKHR vk_surface;
    finite_render_type renderMode;
};

struct finite_render_info *finite_render_info_create(enum finite_render_type *renderMode, char *wayland_device, uint32_t version);
int finite_render_backend_set(int *renderMode);
uint32_t finite_render_create_version(int major, int minor, int patch);
void finite_render_info_remove(struct finite_render_info *render);