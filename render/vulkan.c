#include "render/include/vulkan.h"

/*
    finite_render_vulkan_get_sample_size_from_type    

    Get the VKSampleCountFlag from a finite_render_sample_size enum.
    @returns VkSampleCountFlagBits
*/
VkSampleCountFlagBits finite_render_vulkan_get_sample_size_from_type(enum finite_render_sample_size size) {
    switch (size) {
        case FIN_RENDER_AA_1_BIT:
            return VK_SAMPLE_COUNT_1_BIT;
        case FIN_RENDER_AA_2_BIT:
            return VK_SAMPLE_COUNT_2_BIT;
        case FIN_RENDER_AA_4_BIT:
                return VK_SAMPLE_COUNT_4_BIT;
        case FIN_RENDER_AA_8_BIT:
            return VK_SAMPLE_COUNT_8_BIT;
        case FIN_RENDER_AA_16_BIT:
            return VK_SAMPLE_COUNT_16_BIT;
        case FIN_RENDER_AA_32_BIT:
            return VK_SAMPLE_COUNT_32_BIT;
        case FIN_RENDER_AA_64_BIT:
            return VK_SAMPLE_COUNT_64_BIT;
        default:
            return VK_SAMPLE_COUNT_1_BIT;
    }
}

VkFormat finite_render_vulkan_get_supported_depth_format(VkPhysicalDevice pDevice) {
    VkFormat candidates[] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    for (int i = 0; i < 3; i++) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(pDevice, candidates[i], &props);

        if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return candidates[i];
        }
    }

    return VK_FORMAT_UNDEFINED;
}

/*
    finite_render_vulkan_device_create

    Creates a new VKDevice and attaches a queue and vkPhysicalDevice to the window. 

    @returns VkDevice
*/
VkDevice finite_render_vulkan_device_create(struct finite_render_info *info, struct finite_render_window *window) {
    // TODO: Error checking 
    // grab the physical device
    uint32_t _gpus = 0;
    vkEnumeratePhysicalDevices(info->vk_instance, &_gpus, NULL);
    VkPhysicalDevice gpus[_gpus];
    vkEnumeratePhysicalDevices(info->vk_instance, &_gpus, gpus);

    VkPhysicalDevice pDevice = gpus[0];

    // grab queue family
    uint32_t _families = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &_families, NULL);
    VkQueueFamilyProperties families[_families];
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &_families, families);

    // find a suitable graphics queue
    uint32_t fIndex = NULL;
    for (uint32_t i = 0; i < _families; ++i) {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            fIndex = i;
            break;
        }
    }

    if (!fIndex) {
        // TODO: Error out with finite_log()
        return VK_NULL_HANDLE;
    }

    float priority = 1.0f;
    VkDeviceQueueCreateInfo queue_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = fIndex,
        .queueCount = 1,
        .pQueuePriorities = &priority
    };

    VkDeviceCreateInfo device_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queue_info
    };

    // make Device
    VkDevice device;
    VkResult res = vkCreateDevice(pDevice, &device_info, NULL, &device);
    if (res != VK_SUCCESS) {
        // TODO: Error out with finite_log()
        return VK_NULL_HANDLE;
    }

    vkGetDeviceQueue(device, fIndex, 0, window->vk_queue);
    window->vk_pDevice = pDevice;

    return device;
}

/*
    finite_render_vulkan_surface_create
    
    Create a vulkan surface for the window.
    @return VkSurfaceKHR
*/
VkSurfaceKHR finite_render_vulkan_surface_create(struct finite_render_info *info, struct finite_render_window *window) {
    VkWaylandSurfaceCreateInfoKHR surface_info = {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .display = info->display,
        .surface = window->surface
    };

    VkSurfaceKHR surface;
    VkResult res = vkCreateWaylandSurfaceKHR(info->vk_instance, &surface_info, NULL, surface);
    if (res != VK_SUCCESS) {
        // TODO: Error out with finite_log()
        return VK_NULL_HANDLE;
    }

    return surface;
}

/*
    finite_render_vulkan_swapchain_create

    Create a new swapchain for the window.
*/
struct finite_render_swapchain *finite_render_vulkan_swapchain_create(struct finite_render_window *window) {
    // call VkSwapchainCreateInfoKHR
    VkExtent2D extent = { 1920, 1080 }; // TODO: Allow Devs to set this manually
    VkSwapchainCreateInfoKHR swapchain_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = window->vk_surface,
        .minImageCount = 4, // TODO: Allow devs to set this manuaully
        .imageFormat = VK_FORMAT_R8G8B8A8_SRGB,
        .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage =  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .presentMode = VK_PRESENT_MODE_MAILBOX_KHR // TODO: Allow Devs to set this manually
    };

    struct finite_render_swapchain *swapchain = calloc(1, sizeof(struct finite_render_swapchain)); 
    swapchain->vk_format = VK_FORMAT_R8G8B8A8_SRGB;
    swapchain->vk_depthFormat = VK_FORMAT_UNDEFINED;

    // ensure that a vkDevice is attached to the window
    if (!window->vk_device) {
        window->vk_device = finite_render_vulkan_device_create(window->info, window);
    }
    // call vkCreateSwapchainKHR
    VkResult res = vkCreateSwapchainKHR(window->vk_device, &swapchain_info, NULL, swapchain->vk_swapchain);
    if (res != VK_SUCCESS) {
        // TODO: Error out with finite_log()
        return VK_NULL_HANDLE;
    }

    // setup the finite_render_swapchain
    swapchain->vk_device = window->vk_device;
    vkGetSwapchainImagesKHR(swapchain->vk_device, swapchain->vk_swapchain, &swapchain->imageCount, NULL);
    swapchain->vk_imageHandles = malloc(sizeof(VkImage) * swapchain->imageCount);
    vkGetSwapchainImagesKHR(swapchain->vk_device, swapchain->vk_swapchain, &swapchain->imageCount, swapchain->vk_imageHandles);

    swapchain->vk_imageViews = malloc(sizeof(VkImageView) * swapchain->imageCount);
    for (uint32_t i = 0; i < swapchain->imageCount; i++) {
        VkImageViewCreateInfo view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapchain->vk_imageHandles[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapchain->vk_format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
        };
        VkResult res = vkCreateImageView(swapchain->vk_device, &view_info, NULL, &swapchain->vk_imageViews[i]);
    }

    return swapchain;
}

/*
    finite_render_vulkan_framebuffers_create

    Create a new framebuffer for the renderer. Retuns nothing.
*/
void finite_render_vulkan_framebuffers_create(struct finite_render *render, struct finite_render_image *depth, struct finite_render_image *msaa) {
    struct finite_render_swapchain *swapchain = render->swapchain;
    VkFramebuffer *framebuf = malloc(sizeof(VkFramebuffer) * swapchain->imageCount);
    

    for (uint32_t i = 0; i < render->swapchain->imageCount; i++) {
        VkImageView attach[3];
        uint32_t _attach = 0;

        if (msaa) {
            attach[_attach++] = msaa;
        }

        if (depth) {
            attach[_attach++] = depth;
        }

        attach[_attach] = swapchain->vk_imageViews; // the image views should always 

        VkFramebufferCreateInfo buf_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = render->vk_renderPass,
            .attachmentCount = _attach,
            .pAttachments = attach,
            .width = render->window->vk_extent->width,
            .height = render->window->vk_extent->height,
            .layers = 1
        };

        VkResult res = vkCreateFramebuffer(render->window->vk_device, &buf_info, NULL, render->vk_frameBuf);
        if (res != VK_SUCCESS) {
            // TODO: Error out with finite_log()
            return NULL;
        }
    };
}

/*
    # finite_render_pipeline_layout_default()

    Creates a default pipeline_layout based on the engine being used.
*/

struct finite_render_vulkan_pipeline_layout_config  *finite_render_pipeline_layout_default(struct finite_render *render) {
    struct finite_render_vulkan_pipeline_layout_config *config = calloc(1, sizeof(struct finite_render_vulkan_pipeline_layout_config));
    switch (render->window->info->engine) {
        case FIN_RENDER_ENGINE_UNITY:
            // for Unity we'll want to create a camera
            VkDescriptorSetLayoutBinding default_bindings[] = {
                {
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .pImmutableSamplers = NULL
                },
                {
                    .binding = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .pImmutableSamplers = NULL
                }
            };

            VkPushConstantRange default_pc = {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .offset = 0,
                .size = sizeof(mat4) // matrix from cglm
            };

            config->push_consts = &default_pc;
            config->_pushes = 1;
            config->set_layout = &default_bindings;
            config->_layout = 2;
            break;

        case FIN_RENDER_ENGINE_GODOT:
            // for Godot we'll want to create a camera
            VkDescriptorSetLayoutBinding default_bindings[] = {
                {
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .pImmutableSamplers = NULL
                },
                {
                    .binding = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .pImmutableSamplers = NULL
                }
            };

            VkPushConstantRange default_pc = {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .offset = 0,
                .size = sizeof(mat4) // matrix from cglm
            };

            config->push_consts = &default_pc;
            config->_pushes = 1;
            config->set_layout = &default_bindings;
            config->_layout = 2;
            break;
        
        default:
            config->push_consts = NULL;
            config->_pushes = 0;
            config->set_layout = NULL;
            config->_layout = 0;

        return config;
    }
}

/*
    # finite_render_pipeline_layout_create()

    Creates a pipeline layout with custom options.

    @note Unless developing as a Power User, you don't need to create a new pipeline layout. `finite_render_pipeline_create` will
    automatically create a pipeline layout, assuming you set the pipeline_layout to NULL. 

    Additionally, when called, this function creates a VKPipelineLayout object inside the render's pipeline if its not NULL 
*/
VkPipelineLayout finite_render_pipeline_layout_create(struct finite_render *render, struct finite_render_vulkan_pipeline_layout_config *config) {
    if (!config) {
        // TODO; Error out with finite_log()
        return VK_NULL_HANDLE;
    }

    VkPipelineLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = config->_layout,
        .pSetLayouts = config->set_layout,
        .pushConstantRangeCount = config->_pushes,
        .pPushConstantRanges = config->push_consts
    };

    VkPipelineLayout layout;
    VkResult res = vkCreatePipelineLayout(render->window->vk_device, &layout_info, NULL, layout);
    if (res != VK_SUCCESS) {
        // TODO: Error out with finite_log()
        return VK_NULL_HANDLE;
    }

    if (render->pipeline != NULL) {
        render->pipeline->vk_pipelineLayout = layout;
    }

    return layout;
}