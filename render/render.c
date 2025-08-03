#include "../include/render/render-core.h"
#include "../include/log.h"

FiniteRenderQueueFamilies finite_render_find_queue_families_debug(const char *file, const char *func, int line, VkPhysicalDevice pDevice, VkSurfaceKHR vk_surface) {
    FiniteRenderQueueFamilies indices;
    uint32_t _families = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &_families, NULL);
    finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Searching through %d families.", _families);
    VkQueueFamilyProperties families[_families];
    vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &_families, families);

    for (uint32_t i = 0; i < _families; ++i) {
        FINITE_LOG("Searching through family %d ", i);
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            FINITE_LOG("Found Graphics Channel at %d", i);
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, i, vk_surface, &presentSupport);
        if (presentSupport) {
            FINITE_LOG("Found Present Channel at %d", i);
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
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to find graphics queue group.");
    }

    return indices;
}

bool finite_render_check_extensions_debug(const char *file, const char *func, int line, FiniteRender *render, VkPhysicalDevice pDevice) {
    if (!pDevice) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to check extensions for NULL device.");
        return false;
    }

    FINITE_LOG("Finding extension from device");

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(pDevice, NULL, &extensionCount, NULL);
    FINITE_LOG("Found %d extensions", extensionCount);
    VkExtensionProperties *props = malloc(extensionCount * sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(pDevice, NULL, &extensionCount, props);
    
    bool isFound[render->_devExts];

    for (uint32_t i = 0; i < extensionCount; ++i) {
        // printf("Found extension %s", props[i].extensionName);
        for (size_t j = 0; j < render->_devExts; ++j) {
            if (strcmp(props[i].extensionName, render->required_deviceExtensions[j]) == 0) {
                isFound[j] = true;
                FINITE_LOG_INFO("Found extension %s", props[i].extensionName);
            }
        }
    }

    free(props);

    for (size_t i = 0; i < render->_devExts; ++i) {
        if (!isFound[i]) {
            finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to find the needed extensions.");
            return false;
        }
    }

    FINITE_LOG("Found %d required extension(s)", render->_devExts);
    return true;
}

FiniteRenderSwapchainInfo finite_render_get_swapchain_info_debug(const char *file, const char *func, int line, FiniteRender *render, VkPhysicalDevice pDevice) {
    if (!pDevice) {
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to get swapchain info for NULL device.");
        exit(EXIT_FAILURE);
    }

    if (!render->vk_surface) {
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to get swapchain info for NULL surface.");
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
            finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to allocate memory for mode storage");
        }
        info._modes = _modes;
        vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, render->vk_surface, &_modes, info.modes);

        if (!info.modes) {
            finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to get a valid array of modes.");
        }
    }

    if (_formats >= 0) {
        info.forms = calloc(_formats, sizeof(VkSurfaceFormatKHR));
        if (!info.forms) {
            finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to allocate memory for format storage ");
        }

        info._forms = _formats;
        vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, render->vk_surface, &_formats, info.forms);

        if (!info.forms) {
            finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to get a valid array of forms.");
        }
    }
    FINITE_LOG("Found the requested info.");

    return info;
}

VkSurfaceFormatKHR finite_render_get_best_format_debug(const char *file, const char *func, int line, FiniteRender *render, VkSurfaceFormatKHR *forms, uint32_t _forms) {
    if (_forms < 0 ) {
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to get a valid format when #_forms is not a valid integer");
    }

    for (uint32_t i = 0; i < _forms; ++i) {
        if (forms[i].format == VK_FORMAT_B8G8R8A8_SRGB && forms[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            FINITE_LOG("Using format %d", forms[i].format);
            render->vk_imageForm = forms[i];
            return forms[i]; // use this format
        }
    }

    // otherwise use format[0]
    FINITE_LOG("Using default format %d", forms[0].format);
    render->vk_imageForm = forms[0];
    return forms[0];
}

VkPresentModeKHR finite_render_get_best_present_mode_debug(const char *file, const char *func, int line, FiniteRender *render, VkPresentModeKHR *modes, uint32_t _modes) {
    if (_modes < 0 ) {
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to get valid mode when #_modes is not a valid integer");
    }

    for (uint32_t i = 0; i < _modes; ++i) {
        if (modes[i] == VK_PRESENT_MODE_FIFO_KHR) {
            FINITE_LOG("Using FIFO Mode");
            render->mode = modes[i];
            return modes[i]; // use mailbox
        }
    }

    // otherwise use format[0]
    FINITE_LOG("Using first available mode.");
    render->mode = modes[0];
    return modes[0];
}

VkExtent2D finite_render_get_best_extent(FiniteRender *render, VkSurfaceCapabilitiesKHR *caps, FiniteShell *shell) {
    if (caps->currentExtent.width != UINT32_MAX) {
        FINITE_LOG_INFO("Current extent of %d x %d is acceptable.", caps->currentExtent.width, caps->currentExtent.height);
        render->vk_extent = caps->currentExtent;
        return caps->currentExtent;
    } else {
        // extent is shell.details.extent trust
        VkExtent2D realExtent = {
            .width  = (uint32_t) shell->details->width,
            .height = (uint32_t) shell->details->height
        };

        FINITE_LOG("Checking if new extent of %d x %d is acceptable.", realExtent.width, realExtent.height);

        realExtent.width = realExtent.width < caps->minImageExtent.width ? caps->minImageExtent.width : realExtent.width;
        realExtent.width = realExtent.width > caps->maxImageExtent.width ? caps->maxImageExtent.width : realExtent.width;

        realExtent.height= realExtent.height > caps->maxImageExtent.height ? caps->maxImageExtent.height : realExtent.height;
        realExtent.height = realExtent.height < caps->minImageExtent.height ? caps->minImageExtent.height : realExtent.height;
        FINITE_LOG_INFO("New extent of %d x %d is acceptable.", realExtent.width, realExtent.height);
        render->vk_extent = realExtent;
        return realExtent;
    }
}

bool finite_render_check_device(FiniteRender *render, VkPhysicalDevice pDevice) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(pDevice, &props);
    VkPhysicalDeviceFeatures feats;
    vkGetPhysicalDeviceFeatures(pDevice, &feats);

    FINITE_LOG("Found device named: %s", props.deviceName);

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

void finite_render_record_command_buffer_debug(const char *file, const char *func, int line, FiniteRender *render, uint32_t index) {
    VkCommandBufferBeginInfo start_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0,
        .pInheritanceInfo = NULL
    };

    VkResult res = vkBeginCommandBuffer(render->vk_buffer[render->_currentFrame], &start_info);
    if (res != VK_SUCCESS) {
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to start command buffer recording.");
    }

    VkRenderPassBeginInfo rstart_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = render->vk_renderPass,
        .framebuffer = render->vk_frameBufs[index],
        .renderArea = {
            .offset = {0,0},
            .extent = render->vk_extent
        }
    };

    if (render->withDepth) {
        VkClearValue clear[2] = {
            {
                .color = {{0.25f, 0.25f, 0.25f, 1.0f}}
            },
            {
                .depthStencil = {1.0f, 0}
            }
        };
        rstart_info.clearValueCount = 2,
        rstart_info.pClearValues = clear;
    } else {
        VkClearValue clear = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        rstart_info.clearValueCount = 1;
        rstart_info.pClearValues = &clear;
    }

    vkCmdBeginRenderPass(render->vk_buffer[render->_currentFrame], &rstart_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(render->vk_buffer[render->_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, render->vk_pipeline);

    // add scissor and viewport info
    VkViewport port = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float) render->vk_extent.width,
        .height = (float) render->vk_extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(render->vk_buffer[render->_currentFrame], 0, 1, &port);

    VkRect2D scissor = {
        .offset = {0,0},
        .extent = render->vk_extent
    };
    vkCmdSetScissor(render->vk_buffer[render->_currentFrame], 0, 1, &scissor);

        // if there are vertexBuffers then bind them here

    if (render->_buffers > 0) {
        for (int i = 0; i < render->_buffers; i++) {
            FiniteRenderBuffer currentBuf = render->buffers[i];
            vkCmdBindVertexBuffers(render->vk_buffer[render->_currentFrame], 0, 1, &render->vk_vertexBuf, &currentBuf.vertexOffset);
            if (render->vk_descriptor != NULL) {
                FINITE_LOG("Attempting to bind set %d", render->_currentFrame);
                vkCmdBindDescriptorSets(render->vk_buffer[render->_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, render->vk_layout, 0, 1, &render->vk_descriptor[render->_currentFrame], 0, NULL);
                FINITE_LOG("Done with set %d", render->_currentFrame);
            }
            if (currentBuf._indices == true) {
                vkCmdBindIndexBuffer(render->vk_buffer[render->_currentFrame], render->vk_vertexBuf, currentBuf.indexOffset, VK_INDEX_TYPE_UINT16);
                vkCmdDrawIndexed(render->vk_buffer[render->_currentFrame], currentBuf.indexCount, 1, 0, 0, 0);
            } else {
                vkCmdDraw(render->vk_buffer[render->_currentFrame], currentBuf.vertexCount, 1, 0, 0);
            }
        }
    } else {
        vkCmdDraw(render->vk_buffer[render->_currentFrame], 3, 1, 0, 0);
    }


    // clean up
    vkCmdEndRenderPass(render->vk_buffer[render->_currentFrame]);
    res = vkEndCommandBuffer(render->vk_buffer[render->_currentFrame]);
    if (res != VK_SUCCESS) {
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to Record command buffer");
    }
}

FiniteRenderOneshotBuffer finite_render_begin_onshot_command_debug(const char *file, const char *func, int line, FiniteRender *render) {
    // create a temp pool
    FINITE_LOG("Attempting to create a temporary command pool.");
    VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        .queueFamilyIndex = 0
    };

    VkCommandPool cmd_poole;

    VkResult res = vkCreateCommandPool(render->vk_device, &pool_info, NULL, &cmd_poole);
    if (res != VK_SUCCESS) {
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to create the command pool");
    }

    FINITE_LOG("Created a command pool (%p)", cmd_poole);

    VkCommandBuffer cmd_buffet;

    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = cmd_poole,
        .commandBufferCount = 1
    };
    
    res = vkAllocateCommandBuffers(render->vk_device, &alloc_info, &cmd_buffet);
    if (res != VK_SUCCESS) {
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to create the command buffer");
    }

    FINITE_LOG("Created cmd buffer %p", cmd_buffet);

    VkCommandBufferBeginInfo cmd = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    vkBeginCommandBuffer(cmd_buffet, &cmd);

    FiniteRenderOneshotBuffer buffet = {
        .buffer = cmd_buffet,
        .pool = cmd_poole
    };

    return buffet;
}

void finite_render_finish_onshot_command_debug(const char *file, const char *func, int line, FiniteRender *render, FiniteRenderOneshotBuffer cmd_block) {
    VkCommandBuffer cmd_buffet = cmd_block.buffer;
    VkCommandPool cmd_poole = cmd_block.pool;

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
    VkResult res = vkCreateFence(render->vk_device, &fenceInfo, NULL, &newFence);
    if (res != VK_SUCCESS) {
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to create the fence");
    }

    FINITE_LOG("Created temporary fence %p", newFence);

    vkQueueSubmit(render->vk_graphicsQueue, 1, &info, newFence);
    vkWaitForFences(render->vk_device, 1, &newFence, VK_TRUE, UINT64_MAX);

    vkDestroyFence(render->vk_device, newFence, NULL);
    vkFreeCommandBuffers(render->vk_device, cmd_poole, 1, &cmd_buffet);
    vkDestroyCommandPool(render->vk_device, cmd_poole, NULL);
}

bool finite_render_get_shader_module_debug(const char *file, const char *func, int line, FiniteRender *render, char *code, uint32_t size) {
    if (!render) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Can not apply shader module to NULL renderer");
        return false;
    }

    if (!code) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to load code.");
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
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to create space for new ShaderModule");
        return false;
    }

    VkResult res = vkCreateShaderModule(render->vk_device, &shader, NULL, &render->modules[render->_modules]);
    if (res != VK_SUCCESS) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to create Shader Module");
        return false;
    }

    render->_modules += 1;

    return true;
}

uint32_t finite_render_get_memory_format_debug(const char *file, const char *func, int line, FiniteRender *render, uint32_t filter, VkMemoryPropertyFlags props) {
    if (!render) {
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Can not query memory format with NULL renderer.");
    }

    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(render->vk_pDevice, &memProps);

    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((filter & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & props) == props) {
            return i;
        }
    }

    finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Could not find usable memory type with given filer.");
}

void finite_render_copy_buffer(FiniteRender *render, VkBuffer src, VkBuffer dest, VkDeviceSize size) {
    FiniteRenderOneshotBuffer cmd_block = finite_render_begin_onshot_command(render);
    VkCommandBuffer cmd_buffet = cmd_block.buffer;


    VkBufferCopy cpy = {
        .size = size,
        .srcOffset = 0,
        .dstOffset = 0
    };

    vkCmdCopyBuffer(cmd_buffet, src, dest, 1, &cpy);
    finite_render_finish_onshot_command(render, cmd_block);
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
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(render->vk_device, render->vk_uniformBuf[i], NULL);
        vkFreeMemory(render->vk_device, render->vk_uniformMemory[i], NULL);
    }
    if (render->vk_descPool) {
        free(render->uniformData);
        vkDestroyDescriptorPool(render->vk_device, render->vk_descPool, NULL);
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
    if (render->vk_descriptorLayout != NULL) {
        vkDestroyDescriptorSetLayout(render->vk_device, render->vk_descriptorLayout, NULL);
    }
    if (render->vk_device != NULL) {
        vkDestroyDevice(render->vk_device, NULL);
    }
    vkDestroyInstance(render->vk_instance, NULL);
    finite_draw_cleanup(render->shell);
}