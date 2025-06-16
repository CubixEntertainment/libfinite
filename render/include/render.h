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

#ifndef MAX_FRAMES_IN_FLIGHT
#define MAX_FRAMES_IN_FLIGHT 2
#endif

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
    VkClearValue *vk_clearValues;
};

/*
    details about the pipeline
*/

struct finite_render_pipeline {
    struct finite_render_vulkan_shader_box *shader_box;
    VkPipeline vk_pipeline;
    VkPipelineLayout vk_pipelineLayout;
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
    Additional Details about the swapchain
*/
struct finite_render_swapchain {
    struct finite_render_pipeline *pipeline;
    struct finite_render_image *msaaImage;
    struct finite_render_image *depthImage;
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
    uint32_t fIndex;
    VkSurfaceKHR vk_surface;
    VkDevice vk_device;
    VkPhysicalDevice vk_pDevice;
    VkQueue vk_queue;
    VkExtent2D *vk_extent;
};

struct finite_render_vertex {
    float position[3];
    float normal[3];
    float uv[2];
};

struct finite_render_frame_sync {
    VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
    VkCommandBuffer primaryCommandBuffer;
    VkCommandBuffer secondaryCommandBuffers[4] // up to 4x parallel computing out the box
};

struct finite_render_state {
    VkCommandBuffer vk_cmdBuf;
    uint32_t frameIndex;
    uint32_t imageIndex;
    void *data;
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
    VkFramebuffer *vk_frameBuf;
    enum finite_render_sample_size sampleCount; // for MSAA
    VkCommandPool vk_commandPool;
    VkCommandBuffer *vk_commandBuffer;
    VkQueue vk_presentQueue; // graphics queue is in the window

    struct finite_render_frame_sync frames[MAX_FRAMES_IN_FLIGHT];
    uint32_t currentFrame;

    // Simple profiling (timestamp queries)
    // TODO: Integrate this into finite_log
    VkQueryPool timestampQueryPool;
    uint64_t frameTimestamps[MAX_FRAMES_IN_FLIGHT][2];
};

struct finite_render_info *finite_render_info_create(enum finite_render_type *renderMode, char *wayland_device, char *name, uint32_t version, enum finite_render_engine engine, uint32_t engine_version);
bool finite_render_info_name(struct finite_render_info *info, char *name);
struct finite_render_window *finite_render_window_create(struct finite_render_info *info);
int finite_render_backend_set(int *renderMode);
uint32_t finite_render_create_version(int major, int minor, int patch);
struct finite_render *finite_render_create(struct finite_render_window *window, struct finite_render_swapchain *swapchain, bool withMSAA, enum finite_render_sample_size size);
struct finite_render_image *finite_render_msaa_image_create(struct finite_render *render);
struct finite_render_image *finite_render_depth_image_create(struct finite_render *render);
struct finite_render_pipeline *finite_render_pipeline_create(struct finite_render *render, VkShaderModule *vertShader, VkShaderModule *fragShader, VkPipelineLayout *layout);
void finite_render_info_remove(struct finite_render_info *render);


void finite_render_command_buffer_pool_create(struct finite_render *render);
void finite_render_command_buffer_create(struct finite_render *render);
void finite_render_command_buffer_record(struct finite_render *render, uint32_t index, void (*draw_fn)(VkCommandBuffer));
void finite_render_frame(struct finite_render *render, uint32_t index);

void finite_render_cleanup(struct finite_render *render);