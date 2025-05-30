#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define VK_USE_PLATFORM_WAYLAND_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>
#include <wayland-client.h>
#include <cglm/cglm.h>
#include "protocols/xdg-shell-client-protocol.h"

/*
    Render method actively being used

    Note that Unity is a development only tool
*/
enum finite_render_type {
    FIN_RENDERER_UNITY,
    FIN_RENDERER_VULKAN,
    FIN_RENDERER_OPENGL
};

enum finite_render_engine {
    FIN_RENDER_ENGINE_UNITY,
    FIN_RENDER_ENGINE_GODOT,
    FIN_RENDER_ENGINE_UNKNOWN
};

enum finite_render_sample_size {
    FIN_RENDER_AA_1_BIT,
    FIN_RENDER_AA_2_BIT,
    FIN_RENDER_AA_4_BIT,
    FIN_RENDER_AA_8_BIT,
    FIN_RENDER_AA_16_BIT,
    FIN_RENDER_AA_32_BIT,
    FIN_RENDER_AA_64_BIT
};

/*
    Basic information about the renderer.
*/
struct finite_render_info {
    struct wl_display *display;
    struct wl_surface *surface;
    struct wl_compositor *island; // Islands is the Infinite compositor
    struct xdg_wm_base *base;
    uint32_t version;
    uint16_t engineVersion;
    char clientName;
    enum finite_render_engine engine;
    VkInstance vk_instance;
    VkSurfaceKHR vk_surface;
    enum finite_render_type renderMode;
};

/*
    details about the pipeline
*/

struct finite_render_pipeline {
    VkPipeline vk_pipeline;
    VkPipelineLayout vk_pipelineLayout;
};

/*
    Additional Details about the swapchain
*/
struct finite_render_swapchain {
    struct finite_render_pipeline *pipeline;
    VkDevice vk_device;
    VkSwapchainKHR vk_swapchain;
    uint32_t imageCount;
    VkImage *vk_imageHandles;
    VkImageView *vk_imageViews;
    VkFormat *vk_format;
    VkFormat *vk_depthFormat;

};

/*
    An actual window
*/
struct finite_render_window {
    struct finite_render_info *info;
    struct wl_surface *wl_surface;
    struct xdg_surface *surface;
    struct xdg_toplevel *toplevel;
    VkSurfaceKHR vk_surface;
    VkDevice vk_device;
    VkPhysicalDevice vk_pDevice;
    VkQueue vk_queue;
    VkExtent2D *vk_extent;
};

/*
    Images to be rendered 
*/
struct finite_render_image {
    VkImage vk_image;
    VkDeviceMemory vk_memory;
    VkImageView vk_view;
};

/*
    The actual renderer
*/
struct finite_render {
    struct finite_render_window *window;
    struct finite_render_swapchain *swapchain;
    struct finite_render_pipeline *pipeline;
    uint32_t framebuf_count; // ? unused?
    bool withMSAA;
    VkRenderPass vk_renderPass;
    VkFramebuffer vk_frameBuf;
    enum finite_render_sample_size sampleCount; // for MSAA
};

struct finite_render_info *finite_render_info_create(enum finite_render_type *renderMode, char *wayland_device, char *name, uint32_t version, enum finite_render_engine engine, uint32_t engine_version);
bool finite_render_info_name(struct finite_render_info *info, char *name);
struct finite_render_window *finite_render_window_create(struct finite_render_info *info);
int finite_render_backend_set(int *renderMode);
uint32_t finite_render_create_version(int major, int minor, int patch);
struct finite_render *finite_render_create(struct finite_render_window *window, struct finite_render_swapchain *swapchain, bool withMSAA, enum finite_render_sample_size size);
struct finite_render_image *finite_render_msaa_image_create(struct finite_render *render);
struct finite_render_image *finite_render_depth_image_create(struct finite_render *render);
struct finite_render_pipeline *finite_render_pipeline_create(struct finite_render *render);
void finite_render_frame(struct finite_render *render, struct finite_render_info *info);
void finite_render_info_remove(struct finite_render_info *render);