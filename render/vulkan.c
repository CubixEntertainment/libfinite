#include "render/include/vulkan.h"

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

    return device;
}

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
    swapchain->imageHandles = malloc(sizeof(VkImage) * swapchain->imageCount);
    vkGetSwapchainImagesKHR(swapchain->vk_device, swapchain->vk_swapchain, &swapchain->imageCount, swapchain->imageHandles);

    return swapchain;
}