#include "../include/render/vulkan.h"

FiniteRender *finite_render_init(FiniteShell *shell, char **extensions, char **layers, uint32_t _exts, uint32_t _layers ) {
    if (!shell || !shell->surface) {
        printf("You must create a valid shell with a window before rendering a window.\n");
    }

    FiniteRender *render = calloc(1, sizeof(FiniteRender));

    VkApplicationInfo base_info = {0};
    base_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    base_info.pApplicationName = "Hello World";
    base_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    base_info.pEngineName = "Finite";
    base_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    base_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &base_info;

    char* required_extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
    };
    char* required_layers[] = {"VK_LAYER_KHRONOS_validation"};


    if (extensions != NULL) {
        printf("Loading passed extensions\n");
        create_info.enabledExtensionCount = _exts;
        create_info.ppEnabledExtensionNames = (const char **) extensions;
        render->_exts = _exts;
        render->required_extensions = extensions;
    } else {
        printf("Using default extensions.\n");
        create_info.enabledExtensionCount = 3;
        create_info.ppEnabledExtensionNames = (const char **) required_extensions;
    }

    if (layers != NULL) {
        create_info.enabledLayerCount = _layers;
        create_info.ppEnabledLayerNames =(const char **) layers;
        render->_layers = _layers;
        render->required_layers = layers;
    } else {
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames =(const char **) required_layers;
        render->_layers = 1;
        render->required_layers = required_layers;
    }


    VkResult res = vkCreateInstance(&create_info, NULL, &render->vk_instance);
    if (res != VK_SUCCESS) {
        printf("[Finite] - Unable to create instance.\n");
        return NULL;
    }
    printf("Created Instance %p successfully.\n", render->vk_instance);

    VkWaylandSurfaceCreateInfoKHR surface_info = {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .display = shell->display,
        .surface = shell->isle_surface
    };

    res = vkCreateWaylandSurfaceKHR(render->vk_instance, &surface_info, NULL, &render->vk_surface);
    if (res != VK_SUCCESS) {
        printf("[Finite] - Unable to attach vulkan to surface.\n");
        return NULL;
    }

    printf("Created render surface %p successfully.\n", render->vk_surface);

    return render;
}

void finite_render_create_physical_device(FiniteRender *render) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(render->vk_instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        printf("[Finite] - No devices avilable.\n");
        exit(EXIT_FAILURE);
    }

    printf("%d devices available.\n", deviceCount);

    render->vk_pDevice = VK_NULL_HANDLE;
    VkPhysicalDevice *devs = malloc(deviceCount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(render->vk_instance, &deviceCount, devs);
    for (uint32_t i = 0; i < deviceCount; ++i) {
        if (finite_render_check_device(render, devs[i])) {
            printf("Found ideal device at index %d\n", i);
            render->vk_pDevice = devs[i];
            break;
        }
    }

    if (render->vk_pDevice == VK_NULL_HANDLE) {
        printf("[Finite] - Unable to render with Vulkan with no devices.\n");
        exit(EXIT_FAILURE);
    }

    free(devs);
}

void finite_render_create_device(FiniteRender *render, FiniteRenderQueueFamilies fIndex, uint32_t *uniqueQueueFamilies, char **device_extentsions, uint32_t _ext) {
    float priority = 1.0f;
    VkDeviceQueueCreateInfo *queue_infos = (VkDeviceQueueCreateInfo *)malloc(fIndex._unique * sizeof(VkDeviceQueueCreateInfo));;
    for (uint32_t i = 0; i < fIndex._unique; i++) {
        VkDeviceQueueCreateInfo q = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = uniqueQueueFamilies[i],
            .queueCount = 1,
            .pQueuePriorities = &priority
        };
        queue_infos[i] = q;
        printf("Created queue info for item %d\n", i);
    }

    printf("Created array of %d queue info(s) %p\n", fIndex._unique, queue_infos);

    VkPhysicalDeviceFeatures feats;

    vkGetPhysicalDeviceFeatures(render->vk_pDevice, &feats);   

    VkDeviceCreateInfo device_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = fIndex._unique,
        .pQueueCreateInfos = queue_infos,
        .pEnabledFeatures = &feats,
    };

    const char* requiredDeviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    if (device_extentsions != NULL) {
        device_info.enabledExtensionCount = _ext;
        device_info.ppEnabledExtensionNames = (const char **) device_extentsions;
    } else {
        device_info.enabledExtensionCount = 1;
        device_info.ppEnabledExtensionNames = requiredDeviceExtensions;
    }

    VkResult res = vkCreateDevice(render->vk_pDevice, &device_info, NULL, &render->vk_device);
    if (res != VK_SUCCESS) {
        printf("[Finite] - Unable to create logical device.\n");
        exit(EXIT_FAILURE);
    }

    printf("Created logical device %p\n", render->vk_device);

    vkGetDeviceQueue(render->vk_device , fIndex.graphicsFamily, 0, &render->vk_graphicsQueue);
    vkGetDeviceQueue(render->vk_device , fIndex.presentFamily, 0, &render->vk_presentQueue);
}

void finite_render_create_swapchain(FiniteRender *render, FiniteRenderSwapchainInfo info) {
    uint32_t _images = info.caps.minImageCount + 1;
    if (info.caps.maxImageCount > 0 && _images > info.caps.maxImageCount) {
        _images = info.caps.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchain_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = render->vk_surface,
        .minImageCount = _images,
        .imageFormat = render->vk_imageForm.format,
        .imageColorSpace = render->vk_imageForm.colorSpace,
        .imageExtent = render->vk_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE, // no ownership transferring
        .preTransform = info.caps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = render->mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };

    VkResult res = vkCreateSwapchainKHR(render->vk_device, &swapchain_info, NULL, &render->vk_swapchain);
    if (res != VK_SUCCESS) {
        printf("[Finite] - Unable to create a swapchain.\n");
        exit(EXIT_FAILURE);
    }

    printf("New type VkSwapchain (render.vk_swapchain) (%p)\n", render->vk_swapchain);
}

void finite_render_create_swapchain_images(FiniteRender *render) {
    uint32_t imageCount;
    vkGetSwapchainImagesKHR(render->vk_device, render->vk_swapchain, &imageCount, NULL);
    render->vk_image = (VkImage *)malloc(imageCount * sizeof(VkImage));
    vkGetSwapchainImagesKHR(render->vk_device, render->vk_swapchain, &imageCount, render->vk_image);
    render->vk_view = (VkImageView *)malloc(imageCount * sizeof(VkImageView));

    for (size_t i = 0; i < imageCount; i++) {
        VkImageViewCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = render->vk_image[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = render->vk_imageForm.format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
        };

        VkResult res = vkCreateImageView(render->vk_device, &info, NULL, &render->vk_view[i]);
        if (res != VK_SUCCESS) {
            printf("[Finite] - Unable to create image view %ld\n", i);
            exit(EXIT_FAILURE);
        }
    }

    render->_images = imageCount;

    printf("New type available. Array of %d vk_ImageViews (%p)\n", render->_images, render->vk_view);
}

void finite_render_create_example_render_pass(FiniteRender *render) {
        VkAttachmentDescription colorAttachment = {
        .flags = 0,
        .format = render->vk_imageForm.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };
    
    VkAttachmentReference colorRef = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass = {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = VK_NULL_HANDLE,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorRef,
        .pResolveAttachments = VK_NULL_HANDLE,
        .pDepthStencilAttachment = VK_NULL_HANDLE,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = VK_NULL_HANDLE
    };

    VkSubpassDependency subpass_dep = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = 0
    };

    VkRenderPassCreateInfo renderPass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .flags = 0,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &subpass_dep
    };

    VkResult res = vkCreateRenderPass(render->vk_device, &renderPass_info, NULL, &render->vk_renderPass);
    if (res != VK_SUCCESS) {
        printf("[Finite] - Unable to create render pass | %d \n", res);
        exit(EXIT_FAILURE);
    }

    printf("Render Pass %p created.\n", (void*)render->vk_renderPass);
    printf("New type available. VkRenderPass (%p)\n", render->vk_renderPass);
}

void finite_render_create_framebuffers(FiniteRender *render) {
    VkFramebufferCreateInfo *framebufferCreateInfo = (VkFramebufferCreateInfo *)malloc(render->_images * sizeof(VkFramebufferCreateInfo));
    render->vk_frameBufs = (VkFramebuffer *)malloc(render->_images * sizeof(VkFramebuffer));
    for(uint32_t i = 0; i < render->_images; i++){
		framebufferCreateInfo[i].sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo[i].pNext = VK_NULL_HANDLE;
		framebufferCreateInfo[i].flags = 0;
		framebufferCreateInfo[i].renderPass = render->vk_renderPass;
		framebufferCreateInfo[i].attachmentCount = 1;
		framebufferCreateInfo[i].pAttachments = &(render->vk_view)[i];
		framebufferCreateInfo[i].width = render->vk_extent.width;
		framebufferCreateInfo[i].height = render->vk_extent.height;
		framebufferCreateInfo[i].layers = 1;

		VkResult res = vkCreateFramebuffer(render->vk_device, &framebufferCreateInfo[i], VK_NULL_HANDLE, &render->vk_frameBufs[i]);
        if (res != VK_SUCCESS) {
            printf("Failed to create framebuffer %d\n", i);
            exit(EXIT_FAILURE);
        }
	}

    free(framebufferCreateInfo);

    printf("Created an array of %d framebuffers (%p)\n", render->_images, render->vk_frameBufs);
}

void finite_render_create_pipeline_layout( FiniteRender *render, FiniteRenderPipelineLayoutInfo *layoutInfo) {
    if (!render || layoutInfo == NULL) {
        printf("Unable to create a pipeline layout with NULL inforomation. Render: %p Layout Info: %p\n", render, layoutInfo);
    }

    VkPipelineLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .flags = layoutInfo->flags,
        .pushConstantRangeCount = layoutInfo->_pushRange,
        .pPushConstantRanges = layoutInfo->pushRange,
        .setLayoutCount = layoutInfo->_setConsts,
        .pSetLayouts = layoutInfo->setConsts
    };

    VkResult res = vkCreatePipelineLayout(render->vk_device, &layout_info, NULL, &render->vk_layout);
    if (res != VK_SUCCESS) {
        printf("Unable to create pipeline layout\n");
        exit(EXIT_FAILURE);
    }

    printf("Created a pipeline layout %p\n", render->vk_layout);
}

bool finite_render_add_shader_stage(FiniteRender *render, FiniteRenderShaderStageInfo *stage) {
    if (!render || stage == NULL) {
        printf("Unable to add new shader stage with NULL information. Render: %p Stage Info: %p\n", render, stage);
        return false;
    }

    if (!stage->name) {
        stage->name = "main";
    }

    VkPipelineShaderStageCreateInfo stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = stage->next,
        .flags = stage->flags,
        .module = stage->shader,
        .pName = stage->name,
        .pSpecializationInfo = stage->specializationInfo
    };

    if (stage->stage == FINITE_SHADER_TYPE_VERTEX) {
        printf("Vertex Shader Bit detected\n");
        stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    } else if (stage->stage == FINITE_SHADER_TYPE_FRAGMENT) {
        printf("Fragment Shader Bit detected\n");
        stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    // now add the info to the render array
    printf("Creating shader %d\n", render->stages._stages);
    VkPipelineShaderStageCreateInfo *info = realloc(render->stages.infos, (render->stages._stages + 1) * sizeof(VkPipelineShaderStageCreateInfo));
    if (!info) {
        printf("Unable to allocate memory for FiniteRenderShaderStages\n");
        return false;
    }

    render->stages.infos = info; // assign the reallocated pointer first
    render->stages.infos[render->stages._stages] = stage_info;
    render->stages._stages += 1;

    return true;
}

VkPipelineVertexInputStateCreateInfo finite_render_create_vertex_input(FiniteRender *render, FiniteRenderVertexInputInfo *vertex) {
    if (!render || vertex == NULL) {
        printf("Unable to create vertex input with NULL information. Render: %p Vertex Input Info: %p\n", render, vertex);
        exit(EXIT_FAILURE);
    }

    VkPipelineVertexInputStateCreateInfo input_state_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = vertex->next,
        .flags = vertex->flags,
        .vertexBindingDescriptionCount = vertex->_vertexBindings,
        .pVertexBindingDescriptions = vertex->vertexBindingDescriptions,
        .vertexAttributeDescriptionCount = vertex->_vertexAtributes,
        .pVertexAttributeDescriptions = vertex->vertexAttributeDescriptions
    };

    return input_state_info;
}

VkPipelineInputAssemblyStateCreateInfo finite_render_create_assembly_state(FiniteRender *render, FiniteRenderAssemblyInfo *assemble) {
    if (!render || assemble == NULL) {
        printf("Unable to create input assemble with NULL information. Render: %p Vertex Input Info: %p\n", render, assemble);
        exit(EXIT_FAILURE);
    }

    VkPipelineInputAssemblyStateCreateInfo assemble_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = assemble->next,
        .flags = assemble->flags,
        .topology = assemble->topology,
        .primitiveRestartEnable = assemble->primitiveRestartEnable ? VK_TRUE : VK_FALSE
    };

    return assemble_info;
}

VkPipelineViewportStateCreateInfo finite_render_create_viewport_state(FiniteRender *render, FiniteRenderViewportState *state) {
    if (!render || state == NULL) {
        printf("Unable to create new viewport state with NULL information. Render: %p State Info: %p\n", render, state);
        exit(EXIT_FAILURE);
    }

    VkPipelineViewportStateCreateInfo viewport_state  = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .flags = state->flags,
        .viewportCount = state->_viewports,
        .pViewports = state->viewports,
        .scissorCount = state->_scissors,
        .pScissors = state->scissors
    };

    return viewport_state;
}

VkPipelineRasterizationStateCreateInfo finite_render_create_raster_info(FiniteRender *render, FiniteRenderRasterState *state) {
    if (!render || state == NULL) {
        printf("Unable to create new raster state with NULL information. Render: %p State Info: %p\n", render, state);
        exit(EXIT_FAILURE);
    }

    VkPipelineRasterizationStateCreateInfo raster_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = state->next,
        .flags = state->flags,
        .depthClampEnable = state->depthClampEnable ? VK_TRUE : VK_FALSE,
        .rasterizerDiscardEnable = state->rasterizerDiscardEnable ? VK_TRUE : VK_FALSE,
        .polygonMode = state->polygonMode,
        .cullMode = state->cullMode,
        .frontFace = state->frontFace,
        .depthBiasEnable = state->depthBiasEnable ? VK_TRUE : VK_FALSE,
        .depthBiasConstantFactor = state->depthBiasConstantFactor,
        .depthBiasClamp = state->depthBiasClamp,
        .depthBiasSlopeFactor = state->depthBiasSlopeFactor,
        .lineWidth = state->lineWidth
    };

    return raster_info;
} 

VkPipelineMultisampleStateCreateInfo finite_render_create_multisample_info(FiniteRender *render, FiniteRenderMultisampleStateInfo *info) {
    if (!render || info == NULL) {
        printf("Unable to create new multisample state with NULL information. Render: %p Multisample State Info: %p\n", render, info);
        exit(EXIT_FAILURE);
    }

    VkPipelineMultisampleStateCreateInfo sample_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .flags = info->flags,
        .rasterizationSamples = info->rasterizationSamples,
        .sampleShadingEnable = info->sampleShadingEnable ? VK_TRUE : VK_FALSE,
        .minSampleShading = info->minSampleShading,
        .pSampleMask = info->sampleMask,
        .alphaToCoverageEnable = info->alphaToCoverageEnable ? VK_TRUE : VK_FALSE,
        .alphaToOneEnable = info->alphaToOneEnable ? VK_TRUE : VK_FALSE
    };

    return sample_info;
}

VkPipelineColorBlendAttachmentState finite_render_create_color_blend_attachment(FiniteRenderColorAttachmentInfo *att) {
    if (!att) {
        printf("Unable to create new color blend attachment with NULL information. State Info: %p\n", att);
        exit(EXIT_FAILURE);
    }

    VkPipelineColorBlendAttachmentState blend_state = {
        .blendEnable = att->blendEnable ? VK_TRUE : VK_FALSE,
        .srcColorBlendFactor = att->srcColorBlendFactor,
        .dstColorBlendFactor = att->dstColorBlendFactor,
        .colorBlendOp = att->colorBlendOp,
        .srcAlphaBlendFactor = att->srcAlphaBlendFactor,
        .dstAlphaBlendFactor = att->dstAlphaBlendFactor,
        .alphaBlendOp = att->alphaBlendOp,
        .colorWriteMask = att->colorWriteMask
    };

    return blend_state;
}

VkPipelineColorBlendStateCreateInfo finite_render_create_color_blend_state(FiniteRender *render, VkPipelineColorBlendAttachmentState *att, FiniteRenderColorBlendInfo *blend) {
    if (!render || blend == NULL) {
        printf("Unable to create new color blend state with NULL information. Render: %p State Info: %p\n", render, blend);
        exit(EXIT_FAILURE);
    }

    VkPipelineColorBlendStateCreateInfo blend_state_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .flags = blend->flags,
        .logicOpEnable = blend->logicOpEnable ? VK_TRUE : VK_FALSE,
        .logicOp = blend->logicOp,
        .blendConstants = {
            blend->blendConstants[0],
            blend->blendConstants[1],
            blend->blendConstants[2],
            blend->blendConstants[3]
        }
    };

    if (att != NULL) {
        printf("Using custom type attatchment %p\n", att);
        printf("Props srcBlend: %d(1?) dstBlend %d(0?)\n", att->srcColorBlendFactor, att->dstColorBlendFactor);

        blend_state_info.pAttachments = att;
        blend_state_info.attachmentCount = 1;
        printf("Made color blend data");
    } else {
        blend_state_info.pAttachments = blend->attachments;
        blend_state_info.attachmentCount = blend->_attachments;
    }

    return blend_state_info;
}

bool finite_render_create_graphics_pipeline(FiniteRender *render, VkPipelineCreateFlags flags, VkPipelineVertexInputStateCreateInfo *vertex, VkPipelineInputAssemblyStateCreateInfo *assemble, VkPipelineTessellationStateCreateInfo *tess, VkPipelineViewportStateCreateInfo *port, VkPipelineRasterizationStateCreateInfo *raster, VkPipelineMultisampleStateCreateInfo *sample, VkPipelineColorBlendStateCreateInfo *blend, VkPipelineDynamicStateCreateInfo *dyna)  {
    if (!render) {
        printf("Unable to create new color blend state with NULL render (%p)\n", render);
        return false;
    }

    VkGraphicsPipelineCreateInfo graphics_pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .flags = flags,
        .stageCount = render->stages._stages,
        .pStages = render->stages.infos,
        .pVertexInputState = vertex,
        .pInputAssemblyState = assemble,
        .pTessellationState = tess,
        .pViewportState = port,
        .pRasterizationState = raster,
        .pMultisampleState = sample,
        .pDepthStencilState = VK_NULL_HANDLE, // TODO
        .pColorBlendState = blend,
        .pDynamicState = dyna,
        .layout = render->vk_layout,
        .renderPass = render->vk_renderPass,
        .subpass = 0, // TODO
        .basePipelineHandle = VK_NULL_HANDLE, // TODO
        .basePipelineIndex = -1 // TODO
    };

    VkResult res = vkCreateGraphicsPipelines(render->vk_device, VK_NULL_HANDLE, 1, &graphics_pipeline_info, NULL, &render->vk_pipeline);
    if (res != VK_SUCCESS) {
        printf("Unable to create graphics pipeline.\n");
        return false;
    }

    printf("Created a graphics pipeline (%p)\n", render->vk_pipeline);
    return true;
}

bool finite_render_create_vertex_buffer(FiniteRender *render, FiniteRenderVertexBufferInfo *info, FiniteRenderMemAllocInfo *mem_info, uint64_t vertexSize, FiniteRenderReturnBuffer *rtrn) {
    if (!render) {
        printf("Unable to create new vertex buffer with NULL information Render: %p Info: %p\n", render, info);
        return false;
    }
    
    // create the buffer info. Do note that this is only used if render.vk_vertexBuf is NULL
    VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = info->next,
        .flags = info->flags, 
        .size = info->size, // TODO: keep track of previous size to resize the GPU only buffer (render.vk_vertexBuf)
        .sharingMode = info->sharing,
        .queueFamilyIndexCount = info->_fIndex,
        .pQueueFamilyIndices = info->fIndex
    };

    if (buffer_info.size - vertexSize > 0) {
        buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | info->useFlags; // enforce the required flags
    } else {
        buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | info->useFlags; // enforce the required flags
    }

    FiniteRenderReturnBuffer trn;
    VkMemoryRequirements memRequirements;

    if (rtrn || render->vk_vertexBuf == NULL) {
        VkResult res = vkCreateBuffer(render->vk_device, &buffer_info, NULL, &trn.buf);
        if (res != VK_SUCCESS) {
            printf("[Finite] - Unable to create the Vertex Buffer\n");
            return false;
        }
        
        if (render->vk_vertexBuf == NULL && !rtrn) {
            render->vk_vertexBuf = trn.buf;
        }
    }

    trn.vertexOffset = (VkDeviceSize) vertexSize;

    if (rtrn) {
        rtrn->buf = trn.buf;
        rtrn->vertexOffset = trn.vertexOffset;
        rtrn->indexOffset = vertexSize;
        rtrn->vertexSize = vertexSize;
        rtrn->indexSize = (VkDeviceSize) (buffer_info.size - vertexSize);

        if (rtrn->indexOffset > 0) {
            rtrn->_indices = true;
        }

        // ? devs are responsible for setting rtrn.indices before copying it

        vkGetBufferMemoryRequirements(render->vk_device, rtrn->buf, &memRequirements);
    
    } else {
        // we want to use the buffer that we already have. If it was NULL it's already been defined above
        FiniteRenderBuffer *current_buf;
        VkDeviceSize scale = (VkDeviceSize) (buffer_info.size - vertexSize);
        VkDeviceSize initialOffset = 0;
        current_buf = realloc(render->buffers, sizeof(FiniteRenderBuffer) * (render->_buffers + 1));
        if (!current_buf) {
            printf("Unable to add new FiniteRenderBuffer to array.\n");
            exit(EXIT_FAILURE);
        }
        render->buffers = current_buf;

        if (render->_buffers > 0) {
            FiniteRenderBuffer prev = render->buffers[render->_buffers - 1]; // item render._buffers would be the new item we're adding since we've already realloced by this point
            initialOffset += (prev.indexOffset + prev.indexSize);
        }

        FiniteRenderBuffer dest;
        dest.vertexOffset = initialOffset;
        dest.indexOffset = initialOffset + vertexSize;
        dest.indexSize = scale;
        dest.vertexSize = vertexSize;
        if (dest.indexSize > 0) {
            // we want to have an index
            dest._indices = true;
        }

        render->buffers[render->_buffers] = dest;

        render->_buffers += 1;

        vkGetBufferMemoryRequirements(render->vk_device, render->vk_vertexBuf, &memRequirements);
    }

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = mem_info->next,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = finite_render_get_memory_format(render, memRequirements.memoryTypeBits, mem_info->flags)
    };

    VkResult res = vkAllocateMemory(render->vk_device, &alloc_info, NULL, &trn.mem);
    if (res != VK_SUCCESS) {
        printf("[Finite] - Unable to create the Vertex Buffer Memory\n");
        return false;
    }

    if (rtrn) {
        rtrn->mem = trn.mem;
        vkBindBufferMemory(render->vk_device, rtrn->buf, rtrn->mem, 0);
    } else {
        render->vk_memory = trn.mem;
        vkBindBufferMemory(render->vk_device, render->vk_vertexBuf, render->vk_memory, 0); // render->buffers[render->_buffers - 1] is now the newest item
    }

    return true;
}

bool finite_render_create_command_buffer(FiniteRender *render, bool autocreate, bool isPrimary, uint32_t _buffs) {
    if (autocreate && render->vk_pool == NULL) {
        // create a command pool
        printf("Attempting to auto create a command pool.\n");
        VkCommandPoolCreateInfo pool_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = 0
        };

        VkResult res = vkCreateCommandPool(render->vk_device, &pool_info, NULL, &render->vk_pool);
        if (res != VK_SUCCESS) {
            printf("[Finite] - Unable to create the command pool\n");
            return false;
        }

        printf("Created a command pool (%p)\n", render->vk_pool);
    }

    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = render->vk_pool,
        .level = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY,
        .commandBufferCount = _buffs
    };

    VkResult res = vkAllocateCommandBuffers(render->vk_device, &alloc_info, &render->vk_buffer);
    if (res != VK_SUCCESS) {
        printf("[Finite] - Unable to create the command buffer\n");
        return false;
    }

    printf("Allocated command buffer (%p)\n", render->vk_buffer);
    return true;
}

bool finite_render_create_semaphore(FiniteRender *render) {
    if (!render) {
        printf("Unable to create a semaphore with NULL render (%p)\n", render);
        return false;
    }

    VkSemaphoreCreateInfo signalInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    render->signals = realloc(render->signals, (render->_signals + 1) * sizeof(VkSemaphore));
    VkSemaphore newSignal;
    VkResult res = vkCreateSemaphore(render->vk_device, &signalInfo, NULL, &newSignal);
    if (res != VK_SUCCESS) {
        printf("[Finite] - Unable to create the signal\n");
        return false;
    }

    render->signals[render->_signals] = newSignal;
    render->_signals += 1;

    printf("New signal is at %p.\n", render->signals[render->_signals - 1]);
    return true;
}

bool finite_render_create_fence(FiniteRender *render, VkFenceCreateFlags initialState) {
    if (!render) {
        printf("Unable to create a fence with NULL render (%p)\n", render);
        return false;
    }

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = initialState
    };

    render->fences = realloc(render->fences, (render->_fences + 1) * sizeof(VkFence));
    VkFence newFence;
    VkResult res = vkCreateFence(render->vk_device, &fenceInfo, NULL, &newFence);
    if (res != VK_SUCCESS) {
        printf("[Finite] - Unable to create the fence\n");
        return false;
    }

    render->fences[render->_fences] = newFence;
    render->_fences += 1;

    printf("New Fence is at %p.\n", render->fences[render->_fences - 1]);
    return true;
}

bool finite_render_submit_frame(FiniteRender *render, FiniteRenderSubmitInfo *info, uint32_t fenceId, bool safeExit) {
    if (!render || !info) {
        printf("Unable to submit frame with NULL info. Render: %p Submit Info: %p\n", render, info);
        if (safeExit && render) {
            finite_render_cleanup(render);
            exit(EXIT_FAILURE);
        } else {
            return false;
        }
    }

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = info->next,
        .waitSemaphoreCount = info->_waitSemaphores,
        .pWaitSemaphores = info->waitSemaphores,
        .pWaitDstStageMask = info->waitDstStageMask,
        .commandBufferCount = info->_commandBuffs,
        .pCommandBuffers = info->commandBuffs,
        .signalSemaphoreCount = info->_signalSemaphores,
        .pSignalSemaphores = info->signalSemaphores
    };

    VkFence fence;
    if (fenceId >= 0) {
        if (fenceId > render->_fences) {
            printf("Requested fence %d is out of range.\n", fenceId);
            if (safeExit) {
                finite_render_cleanup(render);
                exit(EXIT_FAILURE);
            } else {
                return false;
            }
        } else {
            fence = render->fences[fenceId];
        }
    } else {
        fence = VK_NULL_HANDLE;
    }

    VkResult res = vkQueueSubmit(render->vk_graphicsQueue, 1, &submit_info, fence);
    if (res != VK_SUCCESS) {
        printf("[Finite] - Unable to connect to the inFlight signal\n");
        return false;
    }

    return true;
}


// attempts to present the given image
// ? returns true if it attempted to run vkQueuePresentKHR. If vkQueuePresentKHR failed, it will still return true.
bool finite_render_present_frame(FiniteRender *render, FiniteRenderPresentInfo *info, bool safeExit) {
    if (!render || !info) {
        printf("Unable to present frame with NULL info. Render: %p Present Info: %p\n", render, info);
        if (safeExit && render) {
            finite_render_cleanup(render);
            exit(EXIT_FAILURE);
        } else {
            return false;
        }
    }

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = info->next,
        .waitSemaphoreCount = info->_waitSemaphores,
        .pWaitSemaphores = info->waitSemaphores,
        .swapchainCount = info->_swapchains,
        .pSwapchains = info->swapchains,
        .pImageIndices = info->imageIndices,
        .pResults = info->results
    };

    // attempt to present the image
    vkQueuePresentKHR(render->vk_presentQueue, &present_info);
    return true;
}