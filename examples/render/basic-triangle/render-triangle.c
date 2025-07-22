/*
    Vulkan drawing with Libfinite SDK example
    Written by Gabriel Thompson <gabriel.thomp@cubixdev.org>
*/

#include <finite/draw.h>
#include <finite/render.h>

int main() {
    printf("Starting...");

    // Create a window to draw the triangle
    FiniteShell *myShell = finite_shell_init("wayland-0");
    finite_window_init(myShell);

    // initialize the renderer
    // ? Passing NULL does NOT set zero extensions. It just tells libfinite to use the default ones
    FiniteRender *render = finite_render_init(myShell, NULL, NULL, 0, 0);

    finite_render_create_physical_device(render);

    // ensure we family queues
    uint32_t uniqueQueueFamilies[2];
    FiniteRenderQueueFamilies fIndex = finite_render_find_queue_families(render->vk_pDevice, render->vk_surface);
    // dedup
    if (fIndex.graphicsFamily != fIndex.presentFamily && fIndex.presentFamily >= 0 ) {
        uniqueQueueFamilies[0] = fIndex.graphicsFamily;
        uniqueQueueFamilies[1] = fIndex.presentFamily;
    } else {
        uniqueQueueFamilies[0] = fIndex.graphicsFamily;
    }

    if (fIndex.graphicsFamily < 0) {
        printf("[Finite] - Unable to find graphics queue group.\n");
        return 1;
    }

    // create the device
    // ? Similarlly to finite_render_init NULL extensions will create the default extensions
    finite_render_create_device(render, fIndex, uniqueQueueFamilies, NULL, 0);

    // now get swapchain details
    FiniteRenderSwapchainInfo info = finite_render_get_swapchain_info(render, render->vk_pDevice);
    finite_render_get_best_format(render, info.forms, info._forms);
    finite_render_get_best_present_mode(render, info.modes, info._modes);
    finite_render_get_best_extent(render, &info.caps, myShell);

    // use those details to make a swapchain
    finite_render_create_swapchain(render, info);

    //create the swapchain images
    finite_render_create_swapchain_images(render);

    // create a render pass
    // ? here I use finite_create_example_render_pass() but you probably want finite_create_default_render_pass() for 3D stuff
    finite_render_create_example_render_pass(render);

    finite_render_create_framebuffers(render);

    // load shaders
    uint32_t vertSize;
    char *vertCode = finite_render_get_shader_code("/path/to/vertex/shader.spv", &vertSize);
    bool success = finite_render_get_shader_module(render, vertCode, vertSize);

    if (!success) {
        printf("Unable to create Vertex Shader Module\n");
        return -1;
    }

    uint32_t fragSize;
    char *fragCode = finite_render_get_shader_code("/path/to/fragment/shader.spv", &fragSize);
    success = finite_render_get_shader_module(render, fragCode, fragSize);

    if (!success) {
        printf("Unable to create Fragment Shader Module\n");
        return -1;
    }

    FiniteRenderPipelineLayoutInfo pipe_info = {
        .flags = 0,
        ._pushRange = 0 ,
        .pushRange = VK_NULL_HANDLE,
        ._setConsts = 0,
        .setConsts = VK_NULL_HANDLE
    };

    finite_render_create_pipeline_layout(render, &pipe_info);

    // add shader modules to render

    FiniteRenderShaderStageInfo vertStage = {
        .flags = 0,
        .stage = FINITE_SHADER_TYPE_VERTEX,
        .shader = render->modules[0],
        .name = "main",
        .specializationInfo = VK_NULL_HANDLE
    };

    finite_render_add_shader_stage(render, &vertStage);

    FiniteRenderShaderStageInfo fragStage = {
        .flags = 0,
        .stage = FINITE_SHADER_TYPE_FRAGMENT,
        .shader = render->modules[1],
        .name = "main",
        .specializationInfo = VK_NULL_HANDLE
    };

    finite_render_add_shader_stage(render, &fragStage);

    // create generic vulkan objects
    FiniteRenderVertexInputInfo vertex = {
        .flags = 0,
        ._vertexBindings = 0,
        ._vertexAtributes = 0,
        .vertexAttributeDescriptions = VK_NULL_HANDLE,
        .vertexBindingDescriptions = VK_NULL_HANDLE
    };

    FiniteRenderAssemblyInfo assemble = {
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = false
    };

    VkViewport viewport = {
        .x = 0,
        .y = 0,
        .width = render->vk_extent.width,
        .height = render->vk_extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkOffset2D off = {
        .x = 0,
        .y = 0
    };

    VkRect2D scissor = {
        .extent = render->vk_extent,
        .offset = off
    };

    FiniteRenderViewportState port = {
        .flags = 0,
        ._viewports = 1,
        ._scissors = 1,
        .viewports = &viewport,
        .scissors = &scissor,
    };

    FiniteRenderRasterState raster = {
        .depthClampEnable = false,
        .rasterizerDiscardEnable = false,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = false,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 1.0f,
        .lineWidth = 1.0f
    };
    
    FiniteRenderMultisampleStateInfo samples = {
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = false,
        .minSampleShading = 1.0f,
        .sampleMask = VK_NULL_HANDLE,
        .alphaToCoverageEnable = false,
        .alphaToOneEnable = false 
    };

    FiniteRenderColorAttachmentInfo blend_att = {
        .blendEnable = false,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    FiniteRenderColorBlendInfo blend_info = {
        .flags = 0,
        .logicOpEnable = false,
        .logicOp = VK_LOGIC_OP_COPY,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
    };

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = dynamicStates,
    };

    // now create the graphics pipline

    VkPipelineVertexInputStateCreateInfo input_state_info = finite_render_create_vertex_input(render, &vertex);
    VkPipelineInputAssemblyStateCreateInfo assemble_info = finite_render_create_assembly_state(render, &assemble);
    VkPipelineViewportStateCreateInfo viewport_info = finite_render_create_viewport_state(render, &port);
    VkPipelineRasterizationStateCreateInfo raster_info = finite_render_create_raster_info(render, &raster);
    VkPipelineMultisampleStateCreateInfo sample_info = finite_render_create_multisample_info(render, &samples);
    VkPipelineColorBlendAttachmentState blend_att_state_info = finite_render_create_color_blend_attachment(&blend_att);
    VkPipelineColorBlendStateCreateInfo blend_state_info = finite_render_create_color_blend_state(render, &blend_att_state_info, &blend_info);

    finite_render_create_graphics_pipeline(render, 0, &input_state_info, &assemble_info, VK_NULL_HANDLE, &viewport_info, &raster_info, &sample_info, &blend_state_info, &dynamicStateInfo);

    // now create the command buffer and autocreate a pool
    // ? for a custom pool, set autocreate to false
    finite_render_create_command_buffer(render, true, true, 1);

    // create two semaphores and one fence
    finite_render_create_semaphore(render); //images available
    finite_render_create_semaphore(render); // renderFinished
    finite_render_create_fence(render, VK_FENCE_CREATE_SIGNALED_BIT);

    // * use pending state!!!

    int state = wl_display_dispatch_pending(myShell->display);
    printf("Success! Dispatch state: %d\n", state);

    // create wayland frame loop
    while (wl_display_dispatch_pending(myShell->display) != -1) {
        // handle custom rendering here
        vkWaitForFences(render->vk_device, 1, &render->fences[0], VK_TRUE, UINT64_MAX);

        uint32_t index;
        vkAcquireNextImageKHR(render->vk_device, render->vk_swapchain, UINT64_MAX, render->signals[0], VK_NULL_HANDLE, &index);

        vkResetCommandBuffer(render->vk_buffer, 0);
        finite_render_record_command_buffer(render, index);

        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        vkResetFences(render->vk_device, 1, &render->fences[0]);

        FiniteRenderSubmitInfo submit_info = {
            ._waitSemaphores = 1,
            .waitSemaphores = &render->signals[0],
            .waitDstStageMask = &waitStage,
            ._commandBuffs = 1,
            .commandBuffs = &render->vk_buffer,
            ._signalSemaphores = 1,
            .signalSemaphores = &render->signals[1]
        };

        // the safeExit param determines whether we want to have finite_render_submit_frame cleanup and exit on failure
        finite_render_submit_frame(render, &submit_info, 0, false);


        VkSwapchainKHR swapchains[] = {render->vk_swapchain};

        FiniteRenderPresentInfo present_info  = {
            ._waitSemaphores = 1,    
            .waitSemaphores = &render->signals[1],
            ._swapchains = 1,
            .swapchains = swapchains,
            .imageIndices = &index,
            .results = NULL
        };

        finite_render_present_frame(render, &present_info, false);
    }
    vkDeviceWaitIdle(render->vk_device);
    finite_render_cleanup(render);
}