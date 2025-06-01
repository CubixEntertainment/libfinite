#include "render/include/vulkan.h"


const char* required_extensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
};

// keep track of the last compositor we found. Since Islands SHOULD be the only compositor running in prod, this makes lookup faster
struct wl_compositor *island;
struct xdg_wm_base *base;

/*
    # islands_registry_handle()

    This handles new connections to the registry so we can find the compositor
*/
static void islands_registry_handle(void* data, struct wl_registry* registry, uint32_t id, const char* interface, uint32_t version) {
    // we want to save the found compositor so that we can attach it to the finite_render_info event
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        island = wl_registry_bind(registry, id, &wl_compositor_interface, 4);
    }
    if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        base = wl_registry_bind(registry, id, &xdg_wm_base_interface, 1);
    }
}

static void islands_registry_handle_remove(void* data, struct wl_registry* registry, uint32_t id);

// create a registry_listener struct for future use
static const struct wl_registry_listener registry_listener = {
    .global = islands_registry_handle,
    .global_remove = islands_registry_handle_remove
};

static char *finite_render_get_engine_from_type(enum finite_render_engine engine) {
    switch (engine) {
        case FIN_RENDER_ENGINE_UNITY:
            return "Unity";
        case FIN_RENDER_ENGINE_GODOT:
            return "Godot";
        default:
            return "Unknown";
    }
}

static uint32_t finite_render_get_memory_type(struct finite_render *render, uint32_t type_filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(render->window->vk_device, &mem_properties);
    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) &&
            (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    // TODO: Error out with finite_log()
    return NULL;
}

/*
    finite_render_create_version

    Creates a valid Vulkan version number. Uses semantic versioning.
*/
uint32_t finite_render_create_version(int major, int minor, int patch) {
    return VK_MAKE_API_VERSION(0,major, minor, patch);
}

/*
    finite_render_info_create()

    Create a new renderer instance. Uses Vulkan by default.
    @param renderMode The rendering mode to use. If NULL it's set to `FIN_RENDERER_VULKAN`
    @param wayland_device The socket the current wayland session is running on. If NULL, wayland reads it as 'wayland-0'
    @param name The name of the app/game being made.
    @param version The version of the app/game being made. 
    @param engine The Engine being used to develop the game. Unless using a commercial engine, this can safely be NULL.
    @param engine_version The version of the engine being used. 
*/
struct finite_render_info *finite_render_info_create(enum finite_render_type *renderMode, char *wayland_device, char *name, uint32_t version, enum finite_render_engine engine, uint32_t engine_version) {
    // by default, finite uses Vulkan. If renderMode is NULL, use vulkan.
    if (!renderMode) {
        renderMode = FIN_RENDERER_VULKAN;
    }

    // if no wayland device is passed use the first available one (wayland-0)
    if (!wayland_device) {
        wayland_device = "wayland-0";
    }

    // If no name is passed, use a default name
    if (!name) {
        name = "Libfinite Window";
    }

    // by default, finite uses 0.0.1 as the version. If version is NULL, use 0.0.1.
    if (!version) {
        version = VK_MAKE_API_VERSION(0,0,0,1);
    }

    // If we can't determine the engine, use Unknown, which disables runtime optimizations (like Render Switching for Unity)
    if (!engine) {
        engine = FIN_RENDER_ENGINE_UNKNOWN;
    }

    // by default, finite uses 0.0.1 as the version. If version is NULL, use 0.0.1.
    if (!engine_version) {
        engine_version = VK_MAKE_API_VERSION(0,0,0,1);
    }

    // create the finite_render_info struct
    struct finite_render_info *render = calloc(1, sizeof(struct finite_render_info));
    // ensure that render is defined
    if (!render) {
        // TODO: Error out with finite_log(Finite_Error, msg)
        return NULL;
    }

    // connect to the display
    render->display = wl_display_connect(wayland_device);
    if (!render->display) {
        // TODO: Error out with finite_log()
        free(wayland_device);
        free(render);
        return NULL;
    }

    // now grab the surface
    struct wl_registry *regis = wl_display_get_registry(render->display);
    if (!regis) {
        // TODO: Error out with finite_log()
        free(render);
        return NULL;
    }    
    wl_registry_add_listener(regis, &registry_listener, NULL);
    // roundtrip to refresh information about the registry. This SHOULD grab the compositor
    wl_display_roundtrip(render->display);
    if (!island || !base) {
        // TODO: Error out with finite_log()
        wl_display_disconnect(render->display);
        free(render);
        free(regis);
        return NULL;
    }
    render->island = island;
    render->base = base;

    struct wl_surface* surface = wl_compositor_create_surface(island);
     if (!surface) {
        // TODO: Error out with finite_log()
        wl_display_disconnect(render->display);
        free(render);
        free(regis);
        return NULL;
    }
    render->surface = surface;

    // create a new Vulkan App information struct
    VkApplicationInfo base_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = NULL,
        .pApplicationName = name,
        .applicationVersion = version,
        .pEngineName = finite_render_get_engine_from_type(engine),
        .engineVersion = engine_version,
        .apiVersion = VK_API_VERSION_1_2
    };

    VkInstanceCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .pApplicationInfo = &base_info,
        .enabledExtensionCount = sizeof(required_extensions) / sizeof(required_extensions[0]),
        .ppEnabledExtensionNames = required_extensions,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL
    };

    // use info to create a new Vulkan Instance
    if (vkCreateInstance(&info, NULL, render->vk_instance) != VK_SUCCESS) {
        // TODO: Error out with finite_log()
        wl_display_disconnect(render->display);
        free(render);
        free(regis);
        return NULL;
    }

    VkWaylandSurfaceCreateInfoKHR island_info = {
        .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
        .pNext = NULL,
        .display = render->display,
        .surface = render->surface
    };

    // use the islands_info to create a new vulkan surface
    if (vkCreateWaylandSurfaceKHR(render->display, &island_info, NULL, render->vk_surface) != VK_SUCCESS) {
        // TODO: Error out with finite_log()
        wl_display_disconnect(render->display);
        free(render);
        free(regis);
        return NULL;
    }

    // finally set some values and return
    render->clientName = name;
    render->version = version;
    render->engine = engine;
    render->engineVersion = engine_version;

    // create default clear values that we can customize later
    // TODO: create finite_set_clear_colors()
    render->vk_clearValues[0] = (VkClearValue){ .color = {{0.1f, 0.1f, 0.1f, 1.0f}} };
    render->vk_clearValues[1] = (VkClearValue){ .depthStencil = {1.0f, 0} };

    return render;
}

/*
    # finite_render_info_name()

    Sets the name of the application.

    @param render_info The render struct to be renamed
    @param name The new name of the application.

    @note `finite_render_info_name()` does not recreate the vkInstance created by `finite_render_info_create()`. In general this
    method should be called to change the name of the app as it appears on UI or when generating a Lua config file.
    
    By setting name to null it will reset to the default name, "Libfinite Window" which may be useful for debug info.

    @returns true if name was successfully changed or the inputted name is the current name.
*/
bool finite_render_info_name(struct finite_render_info *info, char *name) {
    if (!info) {
        // TODO: Error out with finite_log()
        return false;
    }

    if (name == NULL) {
        name = "Libfinite Window";
    }

    // If the name being passed is the current name, do nothing and return true
    char oldName = info->clientName;
    if (strcmp(name, oldName) == 0) {
        // TODO: Log out with finite_log()
        return true;
    }
    info->clientName = name;

    if (strcmp(info->clientName, name) == 0) {
        // TODO: Log out with finitr_log()
        return true;
    }

    return false;
}

/*
    # finite_render_window_create()

    Creates a new window from the renderer.

    @param info The finite_render_info struct associated with the window.
*/
struct finite_render_window *finite_render_window_create(struct finite_render_info *info) {
    if (!info) {
        // TODO: Error out with finite_log()
        return NULL;
    }

    struct finite_render_window *window = calloc(1, sizeof( struct finite_render_window));
    // create the surface from the compositor
    window->wl_surface = wl_compositor_create_surface(info->island);
    window->surface = xdg_wm_base_get_xdg_surface(info->base, info->surface);
    window->toplevel = xdg_surface_get_toplevel(window->surface);

    // try to set fullscreen
    // ? If using Unity, double check the render option we're using
    if (info->engine != FIN_RENDER_ENGINE_UNITY) {
        xdg_toplevel_set_fullscreen(window->toplevel, NULL);
    } else {
        if (info->renderMode != FIN_RENDERER_UNITY) {
            xdg_toplevel_set_fullscreen(window->toplevel, NULL);
        }
    }

    // commit and roundtrip
    wl_surface_commit(window->wl_surface);
    wl_display_roundtrip(info->display);
    // vulkan here
    window->vk_surface = finite_render_vulkan_surface_create(info, window);

    window->info = info;
    return window;
}

/*
    # finite_render_create

    Creates a renderer to handle converting image data to images that can be sent to the swapchain for rendering

    @param window The window you're creating the renderer for.
    @param swapchain The swapchain related to the renderer
    @param withMSAA Toggle Anti-Aliasing. If enabled, ensure `size` is at a value greater than 1 bit.
    @param size The sample count size. If not rendering with MSAA you can leave this NULL.

    @note In order to use the depth_buffer, you'll need to ensure the swapchain didn't set swapchain.vk_depthFormat to VK_FORMAT_UNDEFINED (which it does by default).
    You can manually overwrite this value to support the depth buffer.
*/
struct finite_render *finite_render_create(struct finite_render_window *window, struct finite_render_swapchain *swapchain, bool withMSAA, enum finite_render_sample_size size) {
    if (!window) {
        // TODO: Error out with finite_log()
        return NULL;
    }

    // when not using MSAA set the sample size to 1 Bit
    if (!withMSAA) {
        size = FIN_RENDER_AA_1_BIT;
    }

    // create the new struct
    struct finite_render *render = calloc(1, sizeof(struct finite_render));

    // Do some boilerplate 
    // create two attachments with the second one being optional for withMSAA implementations
    VkAttachmentDescription attachments[3];
    VkAttachmentReference color_ref = {};
    VkAttachmentReference depth_ref = {}; // ? optional
    VkAttachmentReference resolve_ref = {}; // ? MSAA exclusive
    uint32_t _attach = 1;

    attachments[0].format = swapchain->vk_format;
    attachments[0].samples = finite_render_vulkan_get_sample_size_from_type(size);
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = withMSAA ? VK_ATTACHMENT_STORE_OP_DONT_CARE : VK_ATTACHMENT_STORE_OP_STORE; // set OP to Don't Care when resolving with MSAA
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = withMSAA ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    color_ref.attachment = 0;
    color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    bool withDepth = swapchain->vk_depthFormat != VK_FORMAT_UNDEFINED;

    if (withDepth) {
        attachments[1].format = swapchain->vk_depthFormat;
        attachments[1].samples = finite_render_vulkan_get_sample_size_from_type(size);
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        depth_ref.attachment = 1;
        depth_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        _attach++;
    }

    if (withMSAA) {
        attachments[2].format = swapchain->vk_format;
        attachments[2].samples = finite_render_vulkan_get_sample_size_from_type(FIN_RENDER_AA_1_BIT);
        attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[2].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        resolve_ref.attachment = 2;
        resolve_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        _attach++;
    }

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_ref,
        .pResolveAttachments = withMSAA ? &resolve_ref : NULL,
        .pDepthStencilAttachment = withDepth ? &depth_ref : NULL
    };

    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    VkRenderPassCreateInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = _attach,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency
    };

    VkResult res = vkCreateRenderPass(swapchain->vk_device, &render_pass_info, NULL, render->vk_renderPass);
    if (res != VK_SUCCESS) {
        // TODO: Error out with finite_log()
        return NULL;
    }

    render->withMSAA = withMSAA;
    render->sampleCount = size;
    render->swapchain = swapchain;
    render->window = window;
    

    // now try to setup depth and msaa
    if (withDepth) {
        swapchain->depthImage = finite_render_depth_image_create(render);
        if (!swapchain->depthImage) {
            // TODO: Error out with finite_log()
            return NULL;
        }

        // update the swapchain after the image is added
        render->swapchain = swapchain;
    }
    if (withMSAA) {
        swapchain->msaaImage = finite_render_msaa_image_create(render);
        if (!swapchain->msaaImage) {
            // TODO: Error out with finite_log()
            return NULL;
        }

        // update the swapchain after the image is added
        render->swapchain = swapchain;
    }

    return render;
}

/*
    # finite_render_msaa_image_create
    
    For games using Anti_Aliasing, setup MSAA in the image
*/
struct finite_render_image *finite_render_msaa_image_create(struct finite_render *render) {
    struct finite_render_image *image = calloc(1, sizeof(struct finite_render_image));
    struct finite_render_window *window = render->window;
    struct finite_render_swapchain *swapchain = render->swapchain;

    VkImageCreateInfo image_info = {
        .sType  = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = swapchain->vk_format,
        .extent = {
            .width = window->vk_extent->width,
            .height = window->vk_extent->height,
            .depth = 1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = finite_render_vulkan_get_sample_size_from_type(render->sampleCount),
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkResult res = vkCreateImage(window->vk_device, &image_info, NULL, image->vk_image);

    VkMemoryRequirements mem_req;
    vkGetImageMemoryRequirements(window->vk_device, image->vk_image, &mem_req);

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_req.size,
        .memoryTypeIndex = finite_render_get_memory_type(render, mem_req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };

    VkResult res = vkAllocateMemory(window->vk_device, &alloc_info, NULL, image->vk_memory);
    if (res != VK_SUCCESS) {
        // TODO: Error out with finite_log()
        return NULL;
    }

    VkResult res = vkBindImageMemory(window->vk_device, image->vk_image, image->vk_memory, 0);
    if (res != VK_SUCCESS) {
        // TODO: Error out with finite_log()
        return NULL;
    }
    
    VkImageViewCreateInfo view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image->vk_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = swapchain->vk_format,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },  
    };

    VkResult res = vkCreateImageView(window->vk_device, &view_info, NULL, image->vk_view);
    if (res != VK_SUCCESS) {
        // TODO: Error out with finite_log()
        return NULL;
    }

    return image;
}

/*
    # finite_render_depth_image_create

    Creates a depth image.

    @param render The render struct related to the depth image.
*/
struct finite_render_image *finite_render_depth_image_create(struct finite_render *render) {
    struct finite_render_image *image = calloc(1, sizeof(struct finite_render_image));
    struct finite_render_swapchain *swapchain = render->swapchain;
    struct finite_render_window *window = render->window;
    swapchain->vk_depthFormat = finite_render_vulkan_get_supported_depth_format(window->vk_pDevice);

    if (swapchain->vk_depthFormat == VK_FORMAT_UNDEFINED) {
        // TODO: Error out with finite_log()
        return NULL;
    }

    VkImageCreateInfo image_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = swapchain->vk_depthFormat,
        .extent = { window->vk_extent->width, window->vk_extent->height, 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = render->sampleCount,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkResult res = vkCreateImage(window->vk_device, &image_info, NULL, image->vk_image);

    VkMemoryRequirements mem_req;
    vkGetImageMemoryRequirements(window->vk_device, image->vk_image, &mem_req);

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_req.size,
        .memoryTypeIndex = finite_render_get_memory_type(render, mem_req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
    };

    VkResult res = vkAllocateMemory(window->vk_device, &alloc_info, NULL, image->vk_memory);
    if (res != VK_SUCCESS) {
        // TODO: Error out with finite_log()
        return NULL;
    }

    VkResult res = vkBindImageMemory(window->vk_device, image->vk_image, image->vk_memory, 0);
    if (res != VK_SUCCESS) {
        // TODO: Error out with finite_log()
        return NULL;
    }

    VkImageViewCreateInfo view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image->vk_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = swapchain->vk_depthFormat,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },  
    };

    VkResult res = vkCreateImageView(window->vk_device, &view_info, NULL, image->vk_view);
    if (res != VK_SUCCESS) {
        // TODO: Error out with finite_log()
        return NULL;
    }

    return image;
}

/*
    # finite_render_pipeline_create

    Creates a new rendering pipeline.

    @param render The render struct associated with the pipeline
    @param vertShader The Vertex Shader of the pipeline.
    @param fragShader The Fragment Shader of the pipeline
    @param layout The Pipeline Layout of the pipeline. If NULL, `finite_render_pipeline_create()` will create one.

    @returns Returns the finite_render_pipeline sent to the render struct.asm

    @note When creating a new pipeline it's automatically assigned as a member of render struct passed.
*/
struct finite_render_pipeline *finite_render_pipeline_create(struct finite_render *render, VkShaderModule *vertShader, VkShaderModule *fragShader, VkPipelineLayout *layout) {
    if (!layout) {
        // attempt to create basic defaults depending on the engine being used.
        struct finite_render_vulkan_pipeline_layout_config *config = finite_render_pipeline_layout_default(render);
        layout = finite_render_pipeline_layout_create(render, config);
    }

    struct finite_render_pipeline *pipeline = calloc(1, (sizeof(struct finite_render_pipeline)));
    struct finite_render_window *window = render->window;
    if (!pipeline) {
        // TODO: Error out with finite_log()
        return NULL;
    }

    // prepare the stages
    VkPipelineShaderStageCreateInfo vert_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShader,
        .pName = "main",
    };
    VkPipelineShaderStageCreateInfo frag_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = vertShader,
        .pName = "main",
    };

    // combine the shader stages into one Stage Info
    VkPipelineShaderStageCreateInfo shader_info[] = { vert_info, frag_info };

    // pre-input setup
    // TODO: grab binding info from user input
    // >> Add optional attributes (e.g., tangents, bone weights) 
    // >> Move this into finite_render_vertex_format_create()
    // >> Add support for interleaved vs. deinterleaved vertex data
    VkVertexInputBindingDescription bindingDesc = finite_render_vulkan_vertex_binding_desc_create();
    VkVertexInputAttributeDescription *attributeDesc = finite_render_vulkan_vertex_attribute_desc_create();

    VkPipelineVertexInputStateCreateInfo input_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDesc,
        .vertexAttributeDescriptionCount = 3,
        .pVertexAttributeDescriptions = attributeDesc
    };

    // assemble input
    VkPipelineInputAssemblyStateCreateInfo assemble_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    // viewport and scissor
    VkViewport viewport = {
        .x = 0,
        .y = 0,
        // cast the width and hieght to a float
        .width = (float) window->vk_extent->width,
        .height = (float) window->vk_extent->height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = window->vk_extent
    };

    VkPipelineViewportStateCreateInfo viewport_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

    // raster
    VkPipelineRasterizationStateCreateInfo raster_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
    };

    // MSAA
    VkPipelineMultisampleStateCreateInfo msaa_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE, //TODO: Allow devs to toggle this
        .rasterizationSamples = finite_render_vulkan_get_sample_size_from_type(render->sampleCount)
    };

    // Depth Buffer
    VkPipelineDepthStencilStateCreateInfo depth_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE
    };

    // Color Blend
    VkPipelineColorBlendAttachmentState colord = {
        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE
    };

    VkPipelineColorBlendStateCreateInfo color_blending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &colord
    };

    // Create graphics pipeline
    VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shader_info,
        .pVertexInputState = &input_info,
        .pInputAssemblyState = &assemble_info,
        .pViewportState = &viewport_info,
        .pRasterizationState = &raster_info,
        .pMultisampleState = &msaa_info,
        .pDepthStencilState = &depth_info,
        .pColorBlendState = &colord,
        .layout = layout,
        .renderPass = render->vk_renderPass,
        .subpass = 0
    };

    VkResult res = vkCreateGraphicsPipelines(window->vk_device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, pipeline->vk_pipeline);
    if (res != VK_SUCCESS) {
        // TODO: Error out with finite_log()
        return NULL;
    }

    pipeline->vk_pipelineLayout = layout;
    return pipeline;
}

/*
    finite_render_info_remove()

    Cleans up the finite_render_info instance
    @param renderer The finite_render_info struct to cleanup
*/
void finite_render_info_remove(struct finite_render_info *render) {
    vkDestroySurfaceKHR(render->vk_instance, render->vk_surface, NULL);
    vkDestroyInstance(render->vk_instance, NULL);
    wl_display_disconnect(render->display);
    free(render);
}

/*
    # finite_render_command_buffer_pool_create()

    Creates a command pool.

    @param render The renderer associated with the command pool.
*/
void finite_render_command_buffer_pool_create(struct finite_render *render) {
    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // TODO: grab from user input
        .queueFamilyIndex = render->window->fIndex
    };

    VkResult res = vkCreateCommandPool(render->window->vk_device, &poolInfo, NULL, render->vk_commandPool);
    if (res != VK_SUCCESS) {
        // TODO: Error out with finite_log()
        return;
    }
}

/*
    # finite_render_command_buffer_create()

    Allocates a new command buffer.

    @param render The renderer assocaited with the command buffer. Must have a valid command pool attached to it.
*/
void finite_render_command_buffer_create(struct finite_render *render, uint32_t frameIndex) {
    // grab the sync frame
    struct finite_render_frame_sync *frame = &render->frames[frameIndex];

    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = render->vk_commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    VkResult res = vkAllocateCommandBuffers(render->window->vk_device, &alloc_info, &frame->primaryCommandBuffer);
    if (res != VK_SUCCESS) {
        // TODO: Error our with finite_log()
        return;
    }

    // support multithreading
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    alloc_info.commandBufferCount = 4;

    VkResult res = vkAllocateCommandBuffers(render->window->vk_device, &alloc_info, &frame->secondaryCommandBuffers);
    if (res != VK_SUCCESS) {
        // TODO: Error our with finite_log()
        return;
    }

}

/*
    # finite_render_command_buffer_record()

    Records commands to the command buffer.

    @param render The renderer assocaited with the command buffer. Must have a valid command buffer attached to it.
    @param frameIndex The index of the current sync frame.
    @param index The index of the current command buffer to focus on. Should be an imageIndex.
    @param draw_fn An optional callback to handle drawing
*/
void finite_render_command_buffer_record(struct finite_render *render, uint32_t frameIndex, uint32_t index, void (*draw_fn)(struct finite_render_state *state), void *data ) {
    struct finite_render_frame_sync *frame = &render->frames[frameIndex];


    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };

    vkBeginCommandBuffer(frame->primaryCommandBuffer, &begin_info);

    VkRenderPassBeginInfo pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = render->vk_renderPass,
        .framebuffer = render->vk_frameBuf[index],
        .renderArea = {
            .offset = {0, 0},
            .extent = render->window->vk_extent
        },
        .clearValueCount = 2,
        .pClearValues = render->window->info->vk_clearValues
    };

    vkCmdBeginRenderPass(frame->primaryCommandBuffer, &pass_info, VK_SUBPASS_CONTENTS_INLINE);

    // use the draw callback if passed
    if (draw_fn) {
        struct finite_render_state state = {
            .vk_cmdBuf = frame->primaryCommandBuffer,
            .imageIndex = index,
            .frameIndex = frameIndex,
            .data = data
        };
        draw_fn(&state);
    }

    vkCmdEndRenderPass(frame->primaryCommandBuffer);
    vkEndCommandBuffer(frame->primaryCommandBuffer);
}

/*
    # finite_render_frame()

    Attempts to render a single frame.

    @param render The renderer assocaited with the command buffer. Must have a valid command buffer and the necessary semaphores and fences attached
    @param frameIndex The index of the current sync frame
    @param index The current imageIndex to render
*/
void finite_render_frame(struct finite_render *render, uint32_t frameIndex, uint32_t index) {
    struct finite_render_frame_sync *frame = &render->frames[frameIndex];

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = frame->imageAvailableSemaphores,
        .pWaitDstStageMask = (VkPipelineStageFlags[]){VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
        .commandBufferCount = 1,
        .pCommandBuffers = render->vk_commandBuffer[index],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = frame->renderFinishedSemaphores
    };

    vkResetFences(render->window->vk_device, 1, &frame->inFlightFences);
    vkQueueSubmit(render->window->vk_queue, 1, &submitInfo, frame->inFlightFences);

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = frame->renderFinishedSemaphores,
        .swapchainCount = 1,
        .pSwapchains = &render->swapchain,
        .pImageIndices = &index
    };

    vkQueuePresentKHR(render->vk_presentQueue, &presentInfo);
}

/*
    # islands_default_render_loop()

    Starts the default render loop which supports up to four parallel processes.

    @note You can pass an optional draw callback to give yourself fine-grained controll of the drawing and secondary buffers without needing to handle everything yourself.

    @param render The renderer assocaited with the command buffer. Must have a valid command pool attached to it.
    @param draw_fn A callback to handle drawing. Passes back a finite_render_state.
    @param data Optional, additional data.
*/
void islands_default_render_loop(struct finite_render *render, void (*draw_fn)(struct finite_render_state *state), void* data) {
    struct finite_render_frame_sync *frame = &render->frames[render->currentFrame];

    vkWaitForFences(render->window->vk_device, 1, &frame->inFlightFences, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(render->window->vk_device, render, UINT64_MAX,frame->imageAvailableSemaphores, VK_NULL_HANDLE, &imageIndex);

    vkResetFences(render->window->vk_device, 1, &frame->inFlightFences);

    // Wrap normal render loop
    finite_render_command_buffer_pool_create(render);
    finite_render_command_buffer_create(render, render->currentFrame);
    finite_render_command_buffer_record(render, render->currentFrame, imageIndex, draw_fn, data);
    islands_submit_frame(render, render->currentFrame, imageIndex);

    render->currentFrame = (render->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

