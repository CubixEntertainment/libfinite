#include "render/memory.h"
#include "log.h"
#include "render.h"
#include <unistd.h>
#include <vulkan/vulkan_core.h>

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
    return 0; // this line never runs but we include it to silence errors.
}

static VkDeviceSize align_up(VkDeviceSize value, VkDeviceSize alignment) {
    return (value + alignment - 1) & ~(alignment - 1);
}

FiniteRenderLinearAllocator *finite_render_create_linear_allocator_debug(const char *file, const char *func, int line, FiniteRender *render, FiniteRenderAllocatorInfo *info) {
    if (!render) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to create an allocator for render NULL.");
        return NULL;
    }

    if (!render->vk_device) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to create an allocator using render device that doesn't exist (did you add a device?)");
        return NULL;
    }

    if (!info || info->size == 0) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to use NULL info or info with no size.");
        return NULL;
    }

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = info->size,
        .memoryTypeIndex = finite_render_get_memory_format(render, info->memoryTypeBits, info->props)
    };

    VkDeviceMemory mem;

    VkResult res = vkAllocateMemory(render->vk_device, &alloc_info, NULL, &mem);
    if (res != VK_SUCCESS) {
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to allocate memory (%s)\n", VkResultToString(res));
        return NULL;
    }

    FiniteRenderLinearAllocator *alloc = calloc(1, sizeof(FiniteRenderLinearAllocator));
    if (!alloc) {
        vkFreeMemory(render->vk_device, mem, NULL);
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to create linear allocator (no memory available)");
        return NULL;
    }

    alloc->memory = mem;
    alloc->nextFreeByte = 0;
    alloc->size = info->size;
    alloc->mapped = NULL;

    if (info->props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        res = vkMapMemory(render->vk_device, mem, 0, info->size, 0, &alloc->mapped);
        if (res != VK_SUCCESS) {
            vkFreeMemory(render->vk_device, mem, NULL);
            free(alloc);
            finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to create linear allocator (no memory available)");
            return NULL;
        }
    }

    return alloc;
}

FiniteRenderLinearAllocObj *finite_render_linear_alloc_debug(const char *file, const char *func, int line, FiniteRender *render, FiniteRenderLinearAllocator *allocator, VkDeviceSize size, VkDeviceSize alignment) {
    if (!render) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to allocate block for render NULL.");
        return NULL;
    }

    if (!render->vk_device) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to allocate block for render device that doesn't exist (did you add a device?)");
        return NULL;
    }

    if (!allocator) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to allocate block with no allocator");
        return NULL;
    }

    if (size == 0 || alignment == 0) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to allocate block for render device with unknown or zero size or alignment (Size: %ld Alignment: %ld)", size, alignment);
        return NULL;
    }
    
    VkDeviceSize aligned = align_up(allocator->nextFreeByte, alignment);
    if (aligned > allocator->size - size) {
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to allocate block (memory requested is beyond what is available)");
        return NULL;
    }

    FiniteRenderLinearAllocObj *block = calloc(1, sizeof(FiniteRenderLinearAllocObj));
    if (!block) {
        finite_log_internal(LOG_LEVEL_FATAL, file, line, func, "Unable to allocate block (no memory available)");
        return NULL;
    }

    block->memory = allocator->memory;
    block->size = size;
    block->offset = aligned;
    block->mapped = NULL;
    
    if (allocator->mapped) {
        block->mapped = (char *)allocator->mapped + aligned;
    }

    allocator->nextFreeByte = aligned + size;

    return block;
}

VkDeviceSize finite_render_get_available_memory_linear_debug(const char *file, const char *func, int line, FiniteRenderLinearAllocator *allocator) {
    return allocator->size - allocator->nextFreeByte;
}

void finite_render_linear_reset(FiniteRenderLinearAllocator *allocator) {
    // invalidate all original data
    allocator->nextFreeByte = 0;
};

void finite_render_linear_alloc_destroy_debug(const char *file, const char *func, int line, FiniteRender *render, FiniteRenderLinearAllocator *allocator) {
    finite_log_internal(LOG_LEVEL_DEBUG, file, line, func, "Cleaned allocators");
    vkFreeMemory(render->vk_device, allocator->memory, NULL);
    free(allocator);
}