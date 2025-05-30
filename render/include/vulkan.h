#include "render.h"

enum finite_render_vulkan_present_mode {
    FINITE_RENDER_VULKAN_IMMEDIATE_MODE,
    FINITE_RENDER_VULKAN_FIFO_MODE,
    FINITE_RENDER_VULKAN_FIFOR_MODE,
    FINITE_RENDER_VULKAN_MAILBOX_MODE
};

/*
    # finite_render_vulkan_pipeline_layout_config

    A struct to configure the pipeline layout. Made available for advanced configuring needs.
    @note Not intended for use outside of specific cases.
*/
struct finite_render_vulkan_pipeline_layout_config {
    VkDescriptorSetLayout *set_layout;
    uint32_t _layout; // variables with _ preceeding them are count ints
    VkPushConstantRange *push_consts;
    uint32_t _pushes;
};

VkSampleCountFlagBits finite_render_vulkan_get_sample_size_from_type(enum finite_render_sample_size size);

VkDevice finite_render_vulkan_device_create(struct finite_render_info *info, struct finite_render_window *window);

VkSurfaceKHR finite_render_vulkan_surface_create(struct finite_render_info *info, struct finite_render_window *window);

void finite_render_vulkan_framebuffers_create(struct finite_render *render, struct finite_render_image *depth, struct finite_render_image *msaa);

struct finite_render_swapchain *finite_render_vulkan_swapchain_create(struct finite_render_window *window);

VkPipelineLayout finite_render_pipeline_layout_create(struct finite_render *render, struct finite_render_vulkan_pipeline_layout_config *config);

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