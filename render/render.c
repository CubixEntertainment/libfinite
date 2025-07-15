#include "../include/render/render-core.h"

FiniteRenderQueueFamilies finite_render_find_queue_families(VkPhysicalDevice pDevice, VkSurfaceKHR vk_surface) {
    FiniteRenderQueueFamilies indices;
    uint32_t _families = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &_families, NULL);
    printf("Searching through %d families.\n", _families);
    VkQueueFamilyProperties families[_families];
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &_families, families);

    for (uint32_t i = 0; i < _families; ++i) {
        printf("Searching through family %d \n", i);
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            printf("Found Graphics Channel at %d\n", i);
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, i, vk_surface, &presentSupport);
        if (presentSupport) {
            printf("Found Present Channel at %d\n", i);
            indices.presentFamily = i;
        }

        if (indices.graphicsFamily >= 0 && indices.presentFamily >= 0) {
            break;
        }
    } 

        // dedup
    if (indices.graphicsFamily != indices.presentFamily && indices.presentFamily >= 0 ) {
        indices._unique = 2;
    } else {
        indices._unique = 1;
    }

    if (indices.graphicsFamily < 0) {
        printf("[Finite] - Unable to find graphics queue group.\n");
        exit(EXIT_FAILURE);
    }

    return indices;
}

bool finite_render_check_extensions(FiniteRender *render, VkPhysicalDevice pDevice) {
    if (!pDevice) {
        printf("Unable to check extensions for NULL device.\n");
        return false;
    }

    printf("Finding extension from device\n");

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(pDevice, NULL, &extensionCount, NULL);
    printf("Found %d extensions\n", extensionCount);
    VkExtensionProperties *props = malloc(extensionCount * sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(pDevice, NULL, &extensionCount, props);
    
    bool isFound[render->_devExts];

    for (uint32_t i = 0; i < extensionCount; ++i) {
        // printf("Found extension %s\n", props[i].extensionName);
        for (size_t j = 0; j < render->_devExts; ++j) {
            if (strcmp(props[i].extensionName, render->required_deviceExtensions[j]) == 0) {
                isFound[j] = true;
                printf("Found extension %s\n", props[i].extensionName);
            }
        }
    }

    free(props);

    for (size_t i = 0; i < render->_devExts; ++i) {
        if (!isFound[i]) {
            printf("Unable to find the needed extensions.\n");
            return false;
        }
    }

    printf("Found %d required extension(s)\n", render->_devExts);
    return true;
}

FiniteRenderSwapchainInfo finite_render_get_swapchain_info(FiniteRender *render, VkPhysicalDevice pDevice) {
    if (!pDevice) {
        printf("Unable to get swapchain info for NULL device.\n");
        exit(EXIT_FAILURE);
    }

    if (!render->vk_surface) {
        printf("Unable to get swapchain info for NULL surface.\n");
        exit(EXIT_FAILURE);
    }
    
    FiniteRenderSwapchainInfo info;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice, render->vk_surface, &info.caps);

    uint32_t _formats;
    uint32_t _modes;
    vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, render->vk_surface, &_modes, NULL);
    vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, render->vk_surface, &_formats, NULL);

    if (_modes >= 0) {
        info.modes = calloc(_modes, sizeof(VkPresentModeKHR));
        if (!info.modes) {
            printf("Unable to allocate memory for mode storage \n");
            exit(EXIT_FAILURE);
        }
        info._modes = _modes;
        vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, render->vk_surface, &_modes, info.modes);

        if (!info.modes) {
            printf("Unable to get a valid array of modes.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (_formats >= 0) {
        info.forms = calloc(_formats, sizeof(VkSurfaceFormatKHR));
        if (!info.forms) {
            printf("Unable to allocate memory for format storage \n");
            exit(EXIT_FAILURE);
        }

        info._forms = _formats;
        vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, render->vk_surface, &_formats, info.forms);

        if (!info.forms) {
            printf("Unable to get a valid array of forms.\n");
            exit(EXIT_FAILURE);
        }
    }
    printf("Found the requested info.\n");

    return info;
}

VkSurfaceFormatKHR finite_render_get_best_format(FiniteRender *render, VkSurfaceFormatKHR *forms, uint32_t _forms) {
    if (_forms < 0 ) {
        printf("Unable to get a valid format when #_forms is not a valid integer");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < _forms; ++i) {
        if (forms[i].format == VK_FORMAT_B8G8R8A8_SRGB && forms[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            printf("Using format %d\n", forms[i].format);
            render->vk_imageForm = forms[i];
            return forms[i]; // use this format
        }
    }

    // otherwise use format[0]
    printf("Using default format %d\n", forms[0].format);
    render->vk_imageForm = forms[0];
    return forms[0];
}

VkPresentModeKHR finite_render_get_best_present_mode(FiniteRender *render, VkPresentModeKHR *modes, uint32_t _modes) {
    if (_modes < 0 ) {
        printf("Unable to get valid mode when #_modes is not a valid integer");
        exit(EXIT_FAILURE);
    }

    for (uint32_t i = 0; i < _modes; ++i) {
        if (modes[i] == VK_PRESENT_MODE_FIFO_KHR) {
            printf("Using FIFO Mode\n");
            render->mode = modes[i];
            return modes[i]; // use mailbox
        }
    }

    // otherwise use format[0]
    printf("Using first available mode.\n");
    render->mode = modes[0];
    return modes[0];
}

VkExtent2D finite_render_get_best_extent(FiniteRender *render, VkSurfaceCapabilitiesKHR *caps, FiniteShell *shell) {
    if (caps->currentExtent.width != UINT32_MAX) {
        printf("Current extent of %d x %d is acceptable.\n", caps->currentExtent.width, caps->currentExtent.height);
        render->vk_extent = caps->currentExtent;
        return caps->currentExtent;
    } else {
        // extent is shell.details.extent trust
        VkExtent2D realExtent = {
            .width  = (uint32_t) shell->details->width,
            .height = (uint32_t) shell->details->height
        };

        printf("Checking if new extent of %d x %d is acceptable.\n", realExtent.width, realExtent.height);

        realExtent.width = realExtent.width < caps->minImageExtent.width ? caps->minImageExtent.width : realExtent.width;
        realExtent.width = realExtent.width > caps->maxImageExtent.width ? caps->maxImageExtent.width : realExtent.width;

        realExtent.height= realExtent.height > caps->maxImageExtent.height ? caps->maxImageExtent.height : realExtent.height;
        realExtent.height = realExtent.height < caps->minImageExtent.height ? caps->minImageExtent.height : realExtent.height;
        printf("New extent of %d x %d is acceptable.\n", realExtent.width, realExtent.height);
        render->vk_extent = realExtent;
        return realExtent;
    }
}

bool finite_render_check_device(FiniteRender *render, VkPhysicalDevice pDevice) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(pDevice, &props);
    VkPhysicalDeviceFeatures feats;
    vkGetPhysicalDeviceFeatures(pDevice, &feats);

    printf("Found device named: %s\n", props.deviceName);

    bool extSupported = finite_render_check_extensions(render, pDevice);
    bool isSwapchainUsable = false;

    if (extSupported) {
        FiniteRenderSwapchainInfo info = finite_render_get_swapchain_info(render, pDevice);
        if (info._forms > 0 && info._modes > 0 ) {
            isSwapchainUsable = true;
        }
    }

    return (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU || props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) && feats.geometryShader && extSupported && isSwapchainUsable;
}

void finite_render_record_command_buffer(FiniteRender *render, uint32_t index) {
    VkCommandBufferBeginInfo start_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,
        .pInheritanceInfo = NULL
    };

    VkResult res = vkBeginCommandBuffer(render->vk_buffer, &start_info);
    if (res != VK_SUCCESS) {
        printf("[Finite] - Unable to start command buffer recording.\n");
        exit(EXIT_FAILURE);
    }

    VkClearValue clear = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

    VkRenderPassBeginInfo rstart_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = render->vk_renderPass,
        .framebuffer = render->vk_frameBufs[index],
        .renderArea = {
            .offset = {0,0},
            .extent = render->vk_extent
        },
        .clearValueCount = 1,
        .pClearValues = &clear,
    };

    vkCmdBeginRenderPass(render->vk_buffer, &rstart_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(render->vk_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render->vk_pipeline);

    // add scissor and viewport info
    VkViewport port = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float) render->vk_extent.width,
        .height = (float) render->vk_extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(render->vk_buffer, 0, 1, &port);

    VkRect2D scissor = {
        .offset = {0,0},
        .extent = render->vk_extent
    };
    vkCmdSetScissor(render->vk_buffer, 0, 1, &scissor);

        // if there are vertexBuffers then bind them here

    if (render->_buffers > 0) {
        for (int i = 0; i < render->_buffers; i++) {
            FiniteRenderBuffer currentBuf = render->buffers[i];
            vkCmdBindVertexBuffers(render->vk_buffer, 0, 1, &render->vk_vertexBuf, &currentBuf.vertexOffset);
            // vkCmdDraw(render->vk_buffer, 3, 1, 0, 0);
            if (currentBuf._indices == true) {
                vkCmdBindIndexBuffer(render->vk_buffer, render->vk_vertexBuf, currentBuf.indexOffset, VK_INDEX_TYPE_UINT16);
                vkCmdDrawIndexed(render->vk_buffer, currentBuf.indexCount, 1, 0, 0, 0);
            } else {
                vkCmdDraw(render->vk_buffer, currentBuf.vertexCount, 1, 0, 0);
            }
        }
    } else {
        vkCmdDraw(render->vk_buffer, 3, 1, 0, 0);
    }


    // clean up
    vkCmdEndRenderPass(render->vk_buffer);
    res = vkEndCommandBuffer(render->vk_buffer);
    if (res != VK_SUCCESS) {
        printf("[Finite] - Unable to Record command buffer\n");
        exit(EXIT_FAILURE);
    }
}

bool finite_render_get_shader_module(FiniteRender *render, char *code, uint32_t size) {
    if (!render) {
        printf("Can not apply shader module to NULL renderer\n");
        return false;
    }

    if (!code) {
        printf("Unable to load code.\n");
        return false;
    }

    VkShaderModuleCreateInfo shader = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .flags = 0,
        .codeSize = size,
        .pCode = (const uint32_t *)code
    };

    // check now for shader modules
    render->modules = realloc(render->modules, ((render->_modules + 1)*sizeof(VkShaderModule)));
    if (!render->modules) {
        printf("Unable to create space for new ShaderModule");
        return false;
    }

    VkResult res = vkCreateShaderModule(render->vk_device, &shader, NULL, &render->modules[render->_modules]);
    if (res != VK_SUCCESS) {
        printf("Unable to create Shader Module\n");
        return false;
    }

    render->_modules += 1;

    return true;
}

uint32_t finite_render_get_memory_format(FiniteRender *render, uint32_t filter, VkMemoryPropertyFlags props) {
    if (!render) {
        printf("Can not query memory format with NULL renderer.\n");
        exit(EXIT_FAILURE);
    }

    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(render->vk_pDevice, &memProps);

    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((filter & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & props) == props) {
            return i;
        }
    }

    printf("Could not find usable memory type with given filer.\n");
    exit(EXIT_FAILURE);
}

void finite_render_copy_buffer(FiniteRender *render, VkBuffer src, VkBuffer dest, VkDeviceSize size) {
    // create a temp pool
    printf("Attempting to create a temporary command pool.\n");
    VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = 0
    };

    VkCommandPool cmd_poole;

    VkResult res = vkCreateCommandPool(render->vk_device, &pool_info, NULL, &cmd_poole);
    if (res != VK_SUCCESS) {
        printf("[Finite] - Unable to create the command pool\n");
        exit(EXIT_FAILURE);
    }

    printf("Created a command pool (%p)\n", cmd_poole);

    VkCommandBuffer cmd_buffet;

    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = cmd_poole,
        .commandBufferCount = 1
    };
    
    res = vkAllocateCommandBuffers(render->vk_device, &alloc_info, &cmd_buffet);
    if (res != VK_SUCCESS) {
        printf("[Finite] - Unable to create the command buffer\n");
        exit(EXIT_FAILURE);
    }

    printf("Created cmd buffer %p\n", cmd_buffet);

    VkCommandBufferBeginInfo cmd = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    vkBeginCommandBuffer(cmd_buffet, &cmd);

    VkBufferCopy cpy = {
        .size = size,
        .srcOffset = 0,
        .dstOffset = 0
    };

    vkCmdCopyBuffer(cmd_buffet, src, dest, 1, &cpy);
    vkEndCommandBuffer(cmd_buffet);

    // record and ship

    VkSubmitInfo info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buffet
    };

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
    };

    VkFence newFence;
    res = vkCreateFence(render->vk_device, &fenceInfo, NULL, &newFence);
    if (res != VK_SUCCESS) {
        printf("[Finite] - Unable to create the fence\n");
        exit(EXIT_FAILURE);
    }

    printf("Created temporary fence %p\n", newFence);

    vkQueueSubmit(render->vk_graphicsQueue, 1, &info, newFence);
    vkWaitForFences(render->vk_device, 1, &newFence, VK_TRUE, UINT64_MAX);
    printf("Done.\n");

    vkDestroyFence(render->vk_device, newFence, NULL);
    vkFreeCommandBuffers(render->vk_device, cmd_poole, 1, &cmd_buffet);
    vkDestroyCommandPool(render->vk_device, cmd_poole, NULL);
}

void finite_render_cleanup(FiniteRender *render) {
    // clean up shaders FIRST
    if (render->modules != NULL) {
        for (int i = 0; i < render->_modules; i++) {
            vkDestroyShaderModule(render->vk_device, render->modules[i], NULL);
        }
    }
    for (int i = 0; i < render->_signals; i++) {
        vkDestroySemaphore(render->vk_device, render->signals[i], NULL);
    }
    for (int i = 0; i < render->_fences; i++) {
        vkDestroyFence(render->vk_device, render->fences[i], NULL);
    }
    if (render->vk_vertexBuf) {
        vkDestroyBuffer(render->vk_device, render->vk_vertexBuf, NULL);
        vkFreeMemory(render->vk_device, render->vk_memory, NULL);
    }
    if (render->vk_pool != NULL) {
        vkDestroyCommandPool(render->vk_device, render->vk_pool, NULL);
    }
    for (int i = 0; i < render->_images; i++) {
        vkDestroyFramebuffer(render->vk_device, render->vk_frameBufs[i], NULL);
    }
    if (render->vk_pipeline) {
        vkDestroyPipelineLayout(render->vk_device, render->vk_layout, NULL);
    }
    if (render->vk_renderPass) {
        vkDestroyRenderPass(render->vk_device, render->vk_renderPass, NULL);
    }
    for (int i = 0; i < render->_images; i++) {
        vkDestroyImageView(render->vk_device, render->vk_view[i], NULL);
    }
    if (render->vk_swapchain != NULL) {
        vkDestroySwapchainKHR(render->vk_device, render->vk_swapchain, NULL);
    }
    if (render->vk_device != NULL) {
        vkDestroyDevice(render->vk_device, NULL);
    }
    vkDestroyInstance(render->vk_instance, NULL);
    finite_draw_cleanup(render->shell);
}