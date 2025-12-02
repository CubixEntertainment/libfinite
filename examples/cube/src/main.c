/*
    Vulkan 3D Drawing with Libfinite SDK example
    Written by Gabriel Thompson <gabriel.thomp@cubixdev.org>
*/

#include <finite/draw.h>
#include <finite/input.h>
#include <finite/render.h>
#include <finite/audio.h>
#include <finite/log.h>
#include <cglm/call.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE


typedef struct Vertex Vertex;
typedef struct UniformBufferObject UniformBufferObject;

struct Vertex {
    vec3 pos;
    vec3 color;
    vec2 textureCoord;
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
    // Front face
    {{-0.5f, -0.5f,  0.5f},  {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 0
    {{0.5f, -0.5f,  0.5f},   {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // 1
    {{0.5f,  0.5f,  0.5f},   {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // 2
    {{-0.5f,  0.5f,  0.5f},  {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, // 3

    // Back face
    {{0.5f, -0.5f, -0.5f},   {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 4
    {{-0.5f, -0.5f, -0.5f},  {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // 5
    {{-0.5f,  0.5f, -0.5f},  {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // 6
    {{0.5f,  0.5f, -0.5f},   {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, // 7

    // Left face
    {{-0.5f, -0.5f, -0.5f},  {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // 8
    {{-0.5f, -0.5f,  0.5f},  {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, // 9
    {{-0.5f,  0.5f,  0.5f},  {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, //10
    {{-0.5f,  0.5f, -0.5f},  {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, //11

    // Right face
    {{0.5f, -0.5f,  0.5f},   {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, //12
    {{0.5f, -0.5f, -0.5f},   {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, //13
    {{0.5f,  0.5f, -0.5f},   {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, //14
    {{0.5f,  0.5f,  0.5f},   {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, //15

    // Top face
    {{-0.5f,  0.5f,  0.5f},  {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, //16
    {{0.5f,  0.5f,  0.5f},   {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, //17
    {{0.5f,  0.5f, -0.5f},   {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, //18
    {{-0.5f,  0.5f, -0.5f},  {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, //19

    // Bottom face
    {{-0.5f, -0.5f, -0.5f},  {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, //20
    {{0.5f, -0.5f, -0.5f},   {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}}, //21
    {{0.5f, -0.5f,  0.5f},   {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, //22
    {{-0.5f, -0.5f,  0.5f},  {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}, //23
};

// ! All indice data must be in uint32 format/
const uint16_t indexData[] = {
    0,1,2,2,3,0,
    4,5,6,6,7,4,
    8,9,10,10,11,8,
    12,13,14,14,15,12,
    16,17,18,18,19,16,
    20,21,22,22,23,20
};

// size of vertices
int _verts = 24;
int _indexes = 36;

bool canSpin = false;

FinitePlaybackDevice *dev;

void *playMusic(void *data) {
    // audio demo for funny
    char *jingle = "/home/ship/Documents/jingle3.wav";
    dev = finite_audio_device_init();

    finite_audio_get_audio_params(jingle, dev);
    // print out the audio duration
    finite_audio_get_audio_duration(dev);

    // use params to init audio
    finite_audio_init_audio(dev, jingle, false);

    // audio is made so now play

    finite_audio_play(dev);
    FINITE_LOG("Done");
    finite_audio_cleanup(dev);
    sleep(1);

    dev = finite_audio_device_init();
    
    if (!dev) {
        printf("Device: %p is unusable", dev);
        exit(EXIT_FAILURE);
    }

    // reset dev params
    snd_pcm_hw_free(dev->device);
    dev = finite_audio_device_init();
    char *audio = "/home/ship/Downloads/Announcement - rangerbts.mp3";

    finite_audio_get_audio_params(audio, dev);
    // print out the audio duration
    finite_audio_get_audio_duration(dev);
    FINITE_LOG("Preparing for playback");
    // use params to init audio
    finite_audio_init_audio(dev, audio, false);
    FINITE_LOG("Preparing for playback2");
    canSpin = true;
    finite_audio_play(dev);

    // clean up when finished
    finite_audio_cleanup(dev);
    return (void *) 1;
}

void updateUniformBuffer(FiniteRender *render, uint32_t current) {
    static struct timespec startTime = {0};
    struct timespec currentTime;
    double time;
    
    if (canSpin) {
        if (startTime.tv_sec == 0 && startTime.tv_nsec == 0) {
            // First call, initialize startTime
            clock_gettime(CLOCK_MONOTONIC, &startTime);
        }

        clock_gettime(CLOCK_MONOTONIC, &currentTime);

        time = (currentTime.tv_sec - startTime.tv_sec) + (currentTime.tv_nsec - startTime.tv_nsec) / 1e9;
    } else {
        time = 0;
    }

    UniformBufferObject ubo = {0};

    vec3 axis = { 0.0f, 0.0f, 1.0f };

    float angle = glm_rad(-90.0f) * time;

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

    // printf("Adr: %p (%d)\n", render->uniformData[render->_currentFrame], render->_currentFrame);

    // map this to the current buffer
    memcpy(render->uniformData[render->_currentFrame], &ubo, sizeof(UniformBufferObject));    
}

int main() {
    finite_log_init(stdout, LOG_LEVEL_DEBUG, true);
    FINITE_LOG("Starting...");

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
    render->withDepth = true; // enable the depth related features

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
        FINITE_LOG_ERROR("Unable to find graphics queue group."); // with error you have return/exit manually
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
    FiniteRenderAttachmentDescriptionInfo colorInfo = {
        .format = render->vk_imageForm.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    FiniteRenderAttachmentDescriptionInfo depthInfo = {
        .format = VK_FORMAT_D32_SFLOAT,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    FiniteRenderAttachmentRefInfo colorRefInfo = {
        .type = FINITE_ATTACHMENT_DESCRIPTOR_COLOR,
        ._attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    FiniteRenderAttachmentRefInfo depthRefInfo = {
        .type = FINITE_ATTACHMENT_DESCRIPTOR_DEPTH,
        ._attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    FiniteRenderSubpassDescriptionInfo subpass_desc_info = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        ._colorAttachments = 1
    };

    FiniteRenderSubpassDependencyInfo subpass_dep_info = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .destSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .destStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = 0,
        .destAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = 0
    };
    
    FiniteRenderSubpassDependencyInfo *subpassDep_infos[1] = {&subpass_dep_info};
    FiniteRenderSubpassDescriptionInfo *subpassDesc_infos[1] = {&subpass_desc_info};
    FiniteRenderAttachmentRefInfo *ref_infos[2] = {&colorRefInfo, &depthRefInfo};
    FiniteRenderAttachmentDescriptionInfo *attachment_infos[2] = {&colorInfo, &depthInfo};

    FiniteRenderRenderPassInfo render_pass_info = {
        .flags = 0,
        ._attachments = 2,
        ._deps = 1,
        ._refs = 2,
        ._subpasses = 1
    };

    finite_render_create_render_pass(render, attachment_infos, ref_infos, subpassDesc_infos, subpassDep_infos, &render_pass_info);

    // load shaders
    uint32_t vertSize;
    char *vertCode = finite_render_get_shader_code("vert.spv", &vertSize);
    bool success = finite_render_get_shader_module(render, vertCode, vertSize);

    if (!success) {
        FINITE_LOG_ERROR("Unable to create Vertex Shader Module");
        return -1;
    }

    uint32_t fragSize;
    char *fragCode = finite_render_get_shader_code("frag.spv", &fragSize);
    success = finite_render_get_shader_module(render, fragCode, fragSize);

    if (!success) {
        printf("Unable to create Fragment Shader Module");
        return -1;
    }

    // create descriptor
    FiniteRenderDescriptorSetLayout uniformLayout = {
        .binding = 0,
        ._descriptors = 1,
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .flags = VK_SHADER_STAGE_VERTEX_BIT,
        .samplers = NULL
    };

    FiniteRenderDescriptorSetLayout imageLayout = {
        .binding = 1,
        ._descriptors = 1,
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .flags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .samplers = NULL
    };

    FiniteRenderDescriptorSetLayout *bindInfos[2] = {&uniformLayout, &imageLayout};

    finite_render_create_descriptor_layout(render, bindInfos, 2);

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
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, pos)
        },

        {
            .binding = 0,
            .location = 1,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, color)
        },
        {
            .binding = 0,
            .location = 2,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(Vertex, textureCoord)
        }
    };

    // create generic vulkan objects
    FiniteRenderVertexInputInfo vertex = {
        .flags = 0,
        ._vertexBindings = 1,
        ._vertexAtributes = 3,
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

    FiniteRenderImageInfo depthImageInfo = {
        .format = VK_FORMAT_D32_SFLOAT,
        .extent = {
            .width = render->vk_extent.width,
            .height = render->vk_extent.height,
            .depth = 1,
        },
        ._mipLevels = 1,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .useFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        ._samples  = VK_SAMPLE_COUNT_1_BIT,
        .sharing = VK_SHARING_MODE_EXCLUSIVE,
        .imageType = VK_IMAGE_TYPE_2D,
        ._layers = 1,
        .layout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    FiniteRenderMemAllocInfo depth_mem_alloc_info = {
        .flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    };

    FiniteRenderImage *depthImage = finite_render_create_image(render, &depthImageInfo, &depth_mem_alloc_info);
    
    FiniteRenderImageViewInfo depth_info = {
        .image = depthImage->textureImage,
        .type = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_D32_SFLOAT,
        .subRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    finite_render_create_view(render, depthImage, &depth_info);

        FiniteRenderImageBarrierInfo depth_wall_info = {
        .old = VK_IMAGE_LAYOUT_UNDEFINED,
        .new = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .srcfIndex = VK_QUEUE_FAMILY_IGNORED,
        .destfIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = depthImage->textureImage,
        .subRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .srcFlags = 0,
        .destFlags = 0
    };

    FiniteRenderPipelineDirections depth_pipeline_directions = {
        .srcFlags = 0,
        .destFlags = 0,
        .depFlags = 0
    };

    finite_render_transition_image_layout(render, &depth_wall_info, VK_FORMAT_D32_SFLOAT, &depth_pipeline_directions);

    VkImageView att_views[2] = {
        VK_NULL_HANDLE, // this is a very odd but passing way to refer to the render.vk_view
        depthImage->textureImageView
    };

    FiniteRenderFramebufferInfo framebuffer_infos = {
        ._attachments = 2,
        .attachments = att_views,
        .width = render->vk_extent.width,
        .height = render->vk_extent.height,
        .layers = 1,
    };

    finite_render_create_framebuffers(render, &framebuffer_infos);

    FiniteRenderTextureInfo texture_info;
    // populate texture_info
    finite_render_create_texture("texture.png", &texture_info, true);
    // create a staging buffer
    FiniteRenderReturnBuffer repoint;

    FiniteRenderBufferInfo texture_buffer_info = {
        .size = texture_info.size,
        .useFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharing = VK_SHARING_MODE_EXCLUSIVE
    };

    FiniteRenderMemAllocInfo texture_mem_alloc_info = {
        .flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    // creates a non-replicated buffer (will always write to a FiniteRenderReturnBuffer)
    bool suc = finite_render_create_generic_buffer(render, &texture_buffer_info, &texture_mem_alloc_info, texture_info.size, &repoint);
    if (!suc) {
        exit(EXIT_FAILURE);
    }

    void *image_data;
    vkMapMemory(render->vk_device, repoint.mem, 0, texture_buffer_info.size, 0, &image_data);
    memcpy(image_data, texture_info.pixels, (size_t) texture_info.size);
    vkUnmapMemory(render->vk_device, repoint.mem);

    // free the texture pixels
    finite_render_destroy_pixels(&texture_info);
    
    FiniteRenderImageInfo image_info = {
        .extent = {
            .width = texture_info.width,
            .height = texture_info.height,
            .depth = 1,
        },
        .imageType = VK_IMAGE_TYPE_2D,
        ._mipLevels = 1,
        ._layers = 1,
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .layout = VK_IMAGE_LAYOUT_UNDEFINED,
        .useFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        ._samples = VK_SAMPLE_COUNT_1_BIT,
        .sharing = VK_SHARING_MODE_EXCLUSIVE
    };

    texture_mem_alloc_info.flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    FiniteRenderImage *image = finite_render_create_image(render, &image_info, &texture_mem_alloc_info);

    FiniteRenderImageBarrierInfo wall_info = {
        .old = VK_IMAGE_LAYOUT_UNDEFINED,
        .new = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcfIndex = VK_QUEUE_FAMILY_IGNORED,
        .destfIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image->textureImage,
        .subRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .srcFlags = 0,
        .destFlags = 0
    };

    FiniteRenderPipelineDirections pipeline_directions = {
        .srcFlags = 0,
        .destFlags = 0,
        .depFlags = 0
    };

    FiniteRenderImageCopyDirections copy_directions = {
        .offset = 0,
        .rowLength = 0,
        .height = 0,
        .subLayers = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .imageOffset = {0,0,0},
        .extent = {
            .width = texture_info.width,
            .height = texture_info.height,
            .depth = 1,
        },
        .destLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .buffer = repoint.buf,
        .image = image->textureImage
    };

    finite_render_transition_image_layout(render, &wall_info, VK_FORMAT_R8G8B8A8_SRGB, &pipeline_directions);
    finite_render_copy_buffer_to_image(render, &copy_directions);

    wall_info.old = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    wall_info.new = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    finite_render_transition_image_layout(render, &wall_info, VK_FORMAT_R8G8B8A8_SRGB, &pipeline_directions);
    
    FiniteRenderImageViewInfo view_2_info = {
        .image = image->textureImage,
        .type = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .subRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    FiniteRenderTextureSamplerInfo sampler_info = {
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .anisotropyEnable = true,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = false,
        .compareEnable = false,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .mipLodBias = 0.0f,
        .minLod = 0.0f,
        .maxLod = 0.0f
    };

    finite_render_create_view(render, image, &view_2_info);

    finite_render_create_sampler(render, image, &sampler_info);






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

    FINITE_LOG("Rendering object %p: vtx=%u, idx=%u, vtxOffset=%lu (%lu), idxOffset=%lu (%lu)", render->buffers, render->buffers[0].vertexCount, render->buffers[0].indexCount, render->buffers[0].vertexOffset,(sizeof(Vertex) * _verts), render->buffers[0].indexOffset,  (sizeof(uint32_t) * _indexes));

    FiniteRenderBufferInfo uniform_buffer_info = {
        .size = sizeof(UniformBufferObject),
        .sharing = VK_SHARING_MODE_EXCLUSIVE
    };

    FiniteRenderMemAllocInfo uniform_alloc_info = {
        .flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    finite_render_create_uniform_buffer(render, &uniform_buffer_info, &uniform_alloc_info);

    
    FiniteRenderDescriptorPoolInfo imageDescriptor = {
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
    };


    FiniteRenderDescriptorPoolInfo *desc_pool_infos[2] = {NULL, &imageDescriptor};

    finite_render_create_descriptor_pool(render, desc_pool_infos, true, 2);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        FiniteRenderWriteSetInfo buffer_write_info = {
                .dstSet = render->vk_descriptor[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
        };

        FiniteRenderWriteSetInfo image_write_info = {
            .dstSet = render->vk_descriptor[i],
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
        };

        FiniteRenderWriteSetInfo *write_infos[2] = {&buffer_write_info, &image_write_info};

        FiniteRenderDescriptorInfo desc_info = {
            .type = FINITE_DESCRIPTOR_MULTI,
            .buffer = render->vk_uniformBuf[i],
            .buffer_offset = 0,
            .buffer_range = sizeof(UniformBufferObject),
            .image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .image_view = image->textureImageView,
            .image_sampler = image->textureSampler
        };


        finite_render_write_to_descriptor(render, write_infos, &desc_info, 2);
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        // create two semaphores and one fence
        finite_render_create_semaphore(render); //images available
        finite_render_create_semaphore(render); // renderFinished
        finite_render_create_fence(render, VK_FENCE_CREATE_SIGNALED_BIT);
    }

    // * use pending state!!!

    int state = wl_display_dispatch_pending(myShell->display);
    FINITE_LOG("Success! Dispatch state: %d", state);

    // multithread here
    pthread_t id;
    pthread_create(&id, NULL, playMusic, &dev);

    // keyboard demo for additional funny
    // FiniteKeyboard *kbd = finite_input_keyboard_init(myShell->display);

    // FiniteKey xKey = finite_key_from_string("X");
    // if (!finite_key_valid(xKey)) {
    //     printf("Unable to poll NULL key");
    //     exit(EXIT_FAILURE);
    // }

    // create wayland frame loop
    while (wl_display_dispatch_pending(myShell->display) != -1) {
        // poll for input
        // finite_input_poll_keys(kbd, myShell);

        // // handle input
        // if (finite_key_pressed(xKey, kbd)) {
        //     printf("Attempting to pause\n");
        //     finite_audio_pause(dev);
        // }

        // with framesInFlight we need to offset where the indexes are. For reference:
        // 0 -> imagesAvailable
        // 1 -> renderFinished
        // so offset is i + (2 * currentFrame) with since its two items.
        render->_currentFrame = (render->_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;


        int currentFence = 0 + (1 * render->_currentFrame);
        int currentSignal = 0 + (2 * render->_currentFrame);

        // printf("Current Fence: %d (%p) \nCurrent Signal: %d (%p)\n", currentFence, render->fences[currentFence], currentSignal, render->signals[currentSignal]);

        // handle custom rendering here
        vkWaitForFences(render->vk_device, 1, &render->fences[currentFence], VK_TRUE, UINT64_MAX);

        vkResetFences(render->vk_device, 1, &render->fences[currentFence]);
        // printf("Current Fence %d (%p) was reset.\nCurrent Signal: %d (%p)\n", currentFence, render->fences[currentFence], currentSignal, render->signals[currentSignal]);

        uint32_t index;
        vkAcquireNextImageKHR(render->vk_device, render->vk_swapchain, UINT64_MAX, render->signals[currentSignal], VK_NULL_HANDLE, &index);

        vkResetCommandBuffer(render->vk_buffer[render->_currentFrame], 0);
        // printf("recording\n");
        finite_render_record_command_buffer(render, index);
        // printf("Attempting to rotate\n");
        updateUniformBuffer(render, index);
        // printf("Rotate finished\n");
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

        // printf("submitting\n");
        // the safeExit param determines whether we want to have finite_render_submit_frame cleanup and exit on failure
        finite_render_submit_frame(render, &submit_info, currentFence, false);


        VkSwapchainKHR swapchains[] = {render->vk_swapchain};

        FiniteRenderPresentInfo present_info  = {
            ._waitSemaphores = 1,    
            .waitSemaphores = &render->signals[currentSignal + 1],
            ._swapchains = 1,
            .swapchains = swapchains,
            .imageIndices = &index,
            .results = NULL
        };

        // printf("presenting\n");
        finite_render_present_frame(render, &present_info, false);
        // render->_currentFrame = (render->_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        // printf("Current Frame: %d\n", render->_currentFrame);
    }
    vkDeviceWaitIdle(render->vk_device);
    FiniteRenderImage *imgs = {image};
    finite_render_cleanup_textures(render, imgs, 1);
    finite_render_cleanup(render);
}