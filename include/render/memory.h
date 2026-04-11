#ifndef __MEMORY_H__
#define __MEMORY_H__
#include <vulkan/vulkan_core.h>
#include "render-core.h"

typedef struct {
    VkDeviceSize size;
    uint32_t memoryTypeBits;
    VkMemoryPropertyFlags props;
} FiniteRenderAllocatorInfo;

typedef struct {
    VkDeviceMemory memory;
    VkDeviceSize size;
    VkDeviceSize nextFreeByte;

    void *mapped;
} FiniteRenderLinearAllocator;

typedef struct {
    VkDeviceMemory memory;
    VkDeviceSize offset;
    VkDeviceSize size;
    void *mapped;
} FiniteRenderLinearAllocObj;

#define finite_render_create_linear_allocator(render, info) finite_render_create_linear_allocator_debug(__FILE__, __func__, __LINE__, render, info )
FiniteRenderLinearAllocator *finite_render_create_linear_allocator_debug(const char *file, const char *func, int line, FiniteRender *render, FiniteRenderAllocatorInfo *info);

#define finite_render_linear_alloc(render, allocator, size, alignment) finite_render_linear_alloc_debug(__FILE__, __func__, __LINE__, render, allocator, size, alignment)
FiniteRenderLinearAllocObj *finite_render_linear_alloc_debug(const char *file, const char *func, int line, FiniteRender *render, FiniteRenderLinearAllocator *allocator, VkDeviceSize size, VkDeviceSize alignment);

#define finite_render_get_available_memory_linear(allocator) finite_render_get_available_memory_linear_debug(__FILE__, __func__, __LINE__, allocator)
VkDeviceSize finite_render_get_available_memory_linear_debug(const char *file, const char *func, int line, FiniteRenderLinearAllocator *allocator);

void finite_render_linear_reset(FiniteRenderLinearAllocator *allocator);

#define finite_render_linear_alloc_destroy(render, allocator) finite_render_linear_alloc_destroy_debug(__FILE__, __func__, __LINE__, render, allocator)
void finite_render_linear_alloc_destroy_debug(const char *file, const char *func, int line, FiniteRender *render, FiniteRenderLinearAllocator *allocator);

#endif