#include "render.h"

enum finite_render_vulkan_present_mode {
    FINITE_RENDER_VULKAN_IMMEDIATE_MODE,
    FINITE_RENDER_VULKAN_
};

VkDevice finite_render_vulkan_device_create(struct finite_render_info *info);

VkSurfaceKHR finite_render_vulkan_surface_create(struct finite_render_info *info, struct finite_render_window *window);

struct finite_render_swapchain *finite_render_vulkan_swapchain_create(struct finite_render_window *window);

// *getNextFrame
// vkAcquireNextImageKHR to grab next frame
// ? UINT64_MAX = infinite wait
// * fences are cpu, semaphores are GPU

// * draw
// submit the workload to a VkQueue
// call vkQueueSubmit when done
// use fences to sync frames

// * present
// create presentInfo
// call vkQueuePresentKHR