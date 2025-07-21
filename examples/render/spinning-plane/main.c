/*
    Vulkan 3D Drawing with Libfinite SDK example
    Written by Gabriel Thompson <gabriel.thomp@cubixdev.org>
*/

#include <finite/draw.h>
#include <finite/input.h>
#include <finite/render.h>
#include <finite/audio.h>
#include <cglm/call.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>


typedef struct Vertex Vertex;
typedef struct UniformBufferObject UniformBufferObject;

struct Vertex {
    vec2 pos;
    vec3 color;
};


/*
    Vulkan expects the data in your structure to be aligned in memory in a specific way, for example:

    Scalars have to be aligned by N (= 4 bytes given 32 bit floats).
    A vec2 must be aligned by 2N (= 8 bytes)
    A vec3 or vec4 must be aligned by 4N (= 16 bytes)
    A nested structure must be aligned by the base alignment of its members rounded up to a multiple of 16.
    A mat4 matrix must have the same alignment as a vec4.

    You can find the full list of alignment requirements in the specification.
    https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap15.html#interfaces-resources-layout
*/
struct UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
};

// in this example we've made the vertex data a global which is generally not a good idea.
const Vertex vertices[] = {
    {{0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
    {{0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.5f, 0.0f, 1.0f}},
    {{-0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
};

// ! All indice data must be in uint32 format/
const uint16_t indexData[] = {
    0,1,2,2,3,0
};

// size of vertices
int _verts = 4;
int _indexes = 6;

void updateUniformBuffer(FiniteRender *render, uint32_t current) {
    static struct timespec startTime = {0};
    struct timespec currentTime;
    double time;

    if (startTime.tv_sec == 0 && startTime.tv_nsec == 0) {
        // First call, initialize startTime
        clock_gettime(CLOCK_MONOTONIC, &startTime);
    }

    clock_gettime(CLOCK_MONOTONIC, &currentTime);

    time = (currentTime.tv_sec - startTime.tv_sec) + (currentTime.tv_nsec - startTime.tv_nsec) / 1e9;

    UniformBufferObject ubo = {0};

    vec3 axis = { 0.0f, 0.0f, 1.0f };
    float angle = glm_rad(90.0f) * time;

    vec3 eye = {2.0f, 2.0f, 2.0f};
    vec3 center = {0.0f, 0.0f, 0.0f};
    vec3 up = {0.0f, 0.0f, 1.0f};

    float fov = glm_rad(45.0f);
    float aspect = render->vk_extent.width / (float) render->vk_extent.height;
    float near = 0.1f;
    float far = 10.0f;

    glm_mat4_identity(ubo.model);
    glm_rotate(ubo.model, angle, axis);

    glm_mat4_identity(ubo.view);
    glm_lookat(eye, center, up, ubo.view);

    glm_mat4_identity(ubo.proj);
    glm_perspective(fov, aspect, near, far, ubo.proj);

    ubo.proj[1][1] *= -1;

    printf("Adr: %p (%d)\n", render->uniformData[render->_currentFrame], render->_currentFrame);

    // map this to the current buffer
    memcpy(render->uniformData[render->_currentFrame], &ubo, sizeof(UniformBufferObject));    
}

int main() {
    printf("Starting...\n");

    // Create a window to draw the triangle
    FiniteShell *myShell = finite_shell_init("wayland-0");
    finite_window_init(myShell);

    // ! In order for your game to be Infinite compliant you can not resize the window. Here I resize it to make execution easier
    FiniteWindowInfo *det = myShell->details;
    int32_t true_width = det->width;
    int32_t true_height = det->height;

    finite_window_size_set(myShell, ((true_width * 20) / 100), ((true_height *25) / 100), ((true_width * 60) / 100), ((true_height *50) / 100));

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

    //create the swapchain imagesubo.proj[1][1] *= -1;
    finite_render_create_swapchain_images(render);

    // create a render pass
    // ? here I use finite_create_example_render_pass() but you probably want finite_create_default_render_pass() for 3D stuff
    finite_render_create_example_render_pass(render);

    finite_render_create_framebuffers(render);

    // load shaders
    uint32_t vertSize;
    char *vertCode = finite_render_get_shader_code("vert.spv", &vertSize);
    bool success = finite_render_get_shader_module(render, vertCode, vertSize);

    if (!success) {
        printf("Unable to create Vertex Shader Module\n");
        return -1;
    }

    uint32_t fragSize;
    char *fragCode = finite_render_get_shader_code("frag.spv", &fragSize);
    success = finite_render_get_shader_module(render, fragCode, fragSize);

    if (!success) {
        printf("Unable to create Fragment Shader Module\n");
        return -1;
    }

    // create descriptor
    FiniteRenderDescriptorSetLayout bindInfo = {
        .binding = 0,
        ._descriptors = 1,
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .flags = VK_SHADER_STAGE_VERTEX_BIT,
        .samplers = NULL
    };

    finite_render_create_descriptor_layout(render, &bindInfo);

    FiniteRenderPipelineLayoutInfo pipe_info = {
        .flags = 0,
        ._pushRange = 0 ,
        .pushRange = VK_NULL_HANDLE,
        ._setConsts = 1,
        .setConsts = &render->vk_descriptorLayout
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

    // now use custom bindings
    VkVertexInputBindingDescription binding = {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    VkVertexInputAttributeDescription attribe[] = {
        {
            .binding = 0,
            .location = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(Vertex, pos)
        },

        {
            .binding = 0,
            .location = 1,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, color)
        }
    };

    // create generic vulkan objects
    FiniteRenderVertexInputInfo vertex = {
        .flags = 0,
        ._vertexBindings = 1,
        ._vertexAtributes = 2,
        .vertexAttributeDescriptions = attribe,
        .vertexBindingDescriptions = &binding
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
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
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
    // this is the total amount of NEW SPACE needed to create the buffer
    // ? DO NOT try to calculate the total size of the buffer to create new space as it will result in errors.
    FiniteRenderBufferInfo vertex_buffer_info = {
        .size = (sizeof(Vertex) * _verts) + (sizeof(uint16_t) * _indexes),
        .useFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharing = VK_SHARING_MODE_EXCLUSIVE
    };

    FiniteRenderMemAllocInfo mem_alloc_info = {
        .flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    bool prog;
    FiniteRenderReturnBuffer point;
    prog = finite_render_create_vertex_buffer(render, &vertex_buffer_info, &mem_alloc_info, sizeof(Vertex) * _verts, &point);
    if (!prog) {
        exit(EXIT_FAILURE);
    }

    // as a dev you must manually map the vertex buffer when using custom vertex
    void *data;
    vkMapMemory(render->vk_device, point.mem, 0, vertex_buffer_info.size, 0, &data);
    memcpy(data, vertices, (size_t) (sizeof(Vertex) * _verts));
    // ! Make sure to offset the data so memcpy doesnt overwrite
    void *index = (char *)data + sizeof(Vertex) *_verts;
    memcpy(index, indexData, (size_t) (sizeof(uint16_t) * _indexes));
    vkUnmapMemory(render->vk_device, point.mem);

    vertex_buffer_info.useFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    mem_alloc_info.flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    prog = finite_render_create_vertex_buffer(render, &vertex_buffer_info, &mem_alloc_info, sizeof(Vertex) * _verts, NULL);
    if (!prog) {
        exit(EXIT_FAILURE);
    }

    finite_render_copy_buffer(render, point.buf, render->vk_vertexBuf, (point.vertexSize + point.indexSize));
    // add count data
    render->buffers[0]._indices = true;
    render->buffers[0].indexCount = _indexes;
    render->buffers[0].vertexCount = _verts;

    printf("Rendering object %p: vtx=%u, idx=%u, vtxOffset=%lu (%lu), idxOffset=%lu (%lu)\n", render->buffers, render->buffers[0].vertexCount, render->buffers[0].indexCount, render->buffers[0].vertexOffset,(sizeof(Vertex) * _verts), render->buffers[0].indexOffset,  (sizeof(uint32_t) * _indexes));

    FiniteRenderBufferInfo uniform_buffer_info = {
        .size = sizeof(UniformBufferObject),
        .sharing = VK_SHARING_MODE_EXCLUSIVE
    };

    FiniteRenderMemAllocInfo uniform_alloc_info = {
        .flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    finite_render_create_uniform_buffer(render, &uniform_buffer_info, &uniform_alloc_info);

    finite_render_create_descriptor_pool(render, NULL, true);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo buffer_info = {
            .buffer = render->vk_uniformBuf[i],
            .offset = 0,
            .range = sizeof(UniformBufferObject)
        };

        FiniteRenderWriteSetInfo write_info = {
            .dstSet = render->vk_descriptor[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            ._descriptors = 1,
            .bufferInfo = &buffer_info,
            .imageInfo = NULL,
            .texelBufferView = NULL
        };

        finite_render_write_to_descriptor(render, &write_info);
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        // create two semaphores and one fence
        finite_render_create_semaphore(render); //images available
        finite_render_create_semaphore(render); // renderFinished
        finite_render_create_fence(render, VK_FENCE_CREATE_SIGNALED_BIT);
    }

    // * use pending state!!!

    int state = wl_display_dispatch_pending(myShell->display);
    printf("Success! Dispatch state: %d\n", state);

    // create wayland frame loop
    while (wl_display_dispatch_pending(myShell->display) != -1) {
        // with framesInFlight we need to offset where the indexes are. For reference:
        // 0 -> imagesAvailable
        // 1 -> renderFinished
        // so offset is i + (2 * currentFrame) with since its two items.

        int currentFence = 0 + (1 * render->_currentFrame);
        int currentSignal = 0 + (2 * render->_currentFrame);

        printf("Current Fence: %d (%p) \nCurrent Signal: %d (%p)\n", currentFence, render->fences[currentFence], currentSignal, render->signals[currentSignal]);

        // handle custom rendering here
        vkWaitForFences(render->vk_device, 1, &render->fences[currentFence], VK_TRUE, UINT64_MAX);

        vkResetFences(render->vk_device, 1, &render->fences[currentFence]);

        uint32_t index;
        vkAcquireNextImageKHR(render->vk_device, render->vk_swapchain, UINT64_MAX, render->signals[currentSignal], VK_NULL_HANDLE, &index);

        vkResetCommandBuffer(render->vk_buffer[render->_currentFrame], 0);
        printf("recording\n");
        finite_render_record_command_buffer(render, index);
        printf("Attempting to rotate\n");
        updateUniformBuffer(render, index);
        printf("Rotate finished\n");
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        FiniteRenderSubmitInfo submit_info = {
            ._waitSemaphores = 1,
            .waitSemaphores = &render->signals[currentSignal],
            .waitDstStageMask = &waitStage,
            ._commandBuffs = 1,
            .commandBuffs = &render->vk_buffer[render->_currentFrame],
            ._signalSemaphores = 1,
            .signalSemaphores = &render->signals[currentSignal + 1]
        };

        printf("submitting\n");
        // the safeExit param determines whether we want to have finite_render_submit_frame cleanup and exit on failure
        finite_render_submit_frame(render, &submit_info, 0, false);


        VkSwapchainKHR swapchains[] = {render->vk_swapchain};

        FiniteRenderPresentInfo present_info  = {
            ._waitSemaphores = 1,    
            .waitSemaphores = &render->signals[currentSignal + 1],
            ._swapchains = 1,
            .swapchains = swapchains,
            .imageIndices = &index,
            .results = NULL
        };

        printf("presenting\n");
        finite_render_present_frame(render, &present_info, false);
        render->_currentFrame = (render->_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        printf("Current Frame: %d\n", render->_currentFrame);
    }
    vkDeviceWaitIdle(render->vk_device);
    finite_render_cleanup(render);
}