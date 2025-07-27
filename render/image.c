#define STB_IMAGE_IMPLEMENTATION
#include "../include/render/stb_image.h"
#include "../include/render/render-image.h"
#include "../include/render/render-core.h"
#include "../include/log.h"

void finite_render_create_texture_debug(const char *rfile, const char *func, int line, const char *file, FiniteRenderTextureInfo *info, bool forceAlpha) {
    stbi_uc *pixels = stbi_load(file, &info->width, &info->height, &info->channels, forceAlpha ? STBI_rgb_alpha : STBI_rgb);
    if (!pixels) {
        finite_log_internal(LOG_LEVEL_ERROR, rfile, line, func, "Unable to load texture at %s", file);
        exit(EXIT_FAILURE);
    }

    info->size = info->width * info->height * 4;
    info->pixels = pixels; // devs must free this at some point.
}

void finite_render_destroy_pixels(FiniteRenderTextureInfo *image) {
    stbi_image_free(image->pixels);
    image->pixels = NULL;
}

void finite_render_cleanup_textures(FiniteRender *render, FiniteRenderImage *imgs, uint32_t _imgs) {
    for (int i = 0; i < _imgs; i++) {
        vkDestroyImage(render->vk_device, imgs[i].textureImage, NULL);
        vkFreeMemory(render->vk_device, imgs[i].textureImageMemory, NULL);
        vkDestroySampler(render->vk_device, imgs[i].textureSampler, NULL);
        vkDestroyImageView(render->vk_device, imgs[i].textureImageView, NULL);
    }
}

FiniteRenderImage *finite_render_create_image_debug(const char *file, const char *func, int line, FiniteRender *render, FiniteRenderImageInfo *info, FiniteRenderMemAllocInfo *mem_info) {
    if (!render) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable create FiniteRender Image");
        exit(EXIT_FAILURE);
    }

    FiniteRenderImage *image = malloc(sizeof(FiniteRenderImage));
    VkImageCreateInfo image_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = info->next,
        .imageType = info->imageType,
        .extent = info->extent,
        .mipLevels = info->_mipLevels,
        .arrayLayers = info->_layers,
        .format = info->format,
        .tiling = info->tiling,
        .initialLayout = info->layout,
        .flags = info->flags,
        .usage = info->useFlags,
        .queueFamilyIndexCount = info->_fIndex,
        .pQueueFamilyIndices = info->fIndex,
        .samples = info->_samples,
        .sharingMode = info->sharing,
    };

    VkResult res = vkCreateImage(render->vk_device, &image_info, NULL, &image->textureImage);
    if (res != VK_SUCCESS) {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unable to create image");
        exit(EXIT_FAILURE);
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(render->vk_device, image->textureImage, &memRequirements);

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = mem_info->next,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = finite_render_get_memory_format(render, memRequirements.memoryTypeBits, mem_info->flags)
    };

    res = vkAllocateMemory(render->vk_device, &alloc_info, NULL, &image->textureImageMemory);
    if (res != VK_SUCCESS) {
        printf("[Finite] - Unable to create the image memory\n");
        return false;
    }

    vkBindImageMemory(render->vk_device, image->textureImage, image->textureImageMemory, 0);

    return image;
}

void finite_render_create_view(FiniteRender *render, FiniteRenderImage *img, FiniteRenderImageViewInfo *info) {
    if (img->textureImage != info->image) {
        printf("Unable to create view with mismatch info. (%p) (%p)\n", img->textureImage, info->image);
    }
    
    VkImageViewCreateInfo view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = info->next,
        .image = info->image,
        .viewType = info->type,
        .format = info->format,
        .subresourceRange = info->subRange,
        .flags = info->flags,
        .components = info->components
    };

    VkResult res = vkCreateImageView(render->vk_device, &view_info, NULL, &img->textureImageView);
    if (res != VK_SUCCESS) {
        printf("Unable to create image view.\n");
        exit(EXIT_FAILURE);
    }
}

void finite_render_create_sampler(FiniteRender *render, FiniteRenderImage *img, FiniteRenderTextureSamplerInfo *info) {
    VkSamplerCreateInfo sampler_info = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = info->magFilter,
        .minFilter = info->minFilter,
        .addressModeU = info->addressModeU,
        .addressModeV = info->addressModeV,
        .addressModeW = info->addressModeW,
        .anisotropyEnable = info->anisotropyEnable ? VK_TRUE : VK_FALSE,
        .maxAnisotropy = info->maxAnisotropy,
        .compareEnable = info->compareEnable ? VK_TRUE : VK_FALSE,
        .compareOp = info->compareOp,
        .minLod = info->minLod,
        .maxLod = info->maxLod,
        .borderColor = info->borderColor,
        .unnormalizedCoordinates = info->unnormalizedCoordinates ? VK_TRUE : VK_FALSE
    };

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(render->vk_pDevice, &properties);

    if (sampler_info.maxAnisotropy > properties.limits.maxSamplerAnisotropy || (info->anisotropyEnable == true && info->maxAnisotropy == 0)) {
        sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    }

    VkResult res = vkCreateSampler(render->vk_device, &sampler_info, NULL, &img->textureSampler);
    if (res != VK_SUCCESS) {
        printf("Unable to create image sampler.\n");
        exit(EXIT_FAILURE);
    }   
}

void finite_render_transition_image_layout_debug(const char *file, const char *func, int line, FiniteRender *render, FiniteRenderImageBarrierInfo *info, VkFormat format, FiniteRenderPipelineDirections *dir) {
    if (!info) {
        printf("Unable to transition NULL image (%p)\n", info);
        exit(EXIT_FAILURE);
    }

    FiniteRenderOneshotBuffer cmd_block = finite_render_begin_onshot_command(render);

    if (info->old == VK_IMAGE_LAYOUT_UNDEFINED && info->new == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        FINITE_LOG("Old is undefined. Moving to Transfer Optimal");        
        info->srcFlags = 0;
        info->destFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
        dir->srcFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dir->destFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (info->old == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && info->new == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        FINITE_LOG("Old is Transfer Optimal. Moving to Read Only");
        info->srcFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
        info->destFlags = VK_ACCESS_SHADER_READ_BIT;
        dir->srcFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dir->destFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (info->old == VK_IMAGE_LAYOUT_UNDEFINED && info->new == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        FINITE_LOG("Old is undefined. Moving to Depth Stencil");
        info->srcFlags = 0;
        info->destFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dir->srcFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dir->destFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    } else {
        finite_log_internal(LOG_LEVEL_ERROR, file, line, func, "Unknown/unsupported transition.");
        exit(EXIT_FAILURE);
    }

    VkImageMemoryBarrier wall = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = info->old,
        .newLayout = info->new,
        .srcQueueFamilyIndex = info->srcfIndex,
        .dstQueueFamilyIndex = info->destfIndex,
        .image = info->image,
        .subresourceRange = info->subRange,
        .srcAccessMask = info->srcFlags,
        .dstAccessMask = info->destFlags 
    };

    if (info->new == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) {
            if (info->subRange.aspectMask != VK_IMAGE_ASPECT_STENCIL_BIT) {
                wall.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
                finite_log_internal(LOG_LEVEL_WARN, file, line, func, "Using stencil, defaulting to stencil bit");
            }
        } else {
            if (info->subRange.aspectMask != VK_IMAGE_ASPECT_DEPTH_BIT) {
                wall.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
                finite_log_internal(LOG_LEVEL_WARN, file, line, func, "Using stencil, defaulting to depth bit");
            }
        }
    }

    vkCmdPipelineBarrier(cmd_block.buffer, dir->srcFlags, dir->destFlags, dir->depFlags, 0, NULL, 0, NULL, 1, &wall);
    finite_render_finish_onshot_command(render, cmd_block);
}

void finite_render_copy_buffer_to_image(FiniteRender *render, FiniteRenderImageCopyDirections *dir) {
    FiniteRenderOneshotBuffer cmd_block = finite_render_begin_onshot_command(render);

    VkBufferImageCopy region = {
        .bufferOffset = dir->offset,
        .bufferRowLength = dir->rowLength,
        .bufferImageHeight = dir->height,
        .imageSubresource = dir->subLayers,
        .imageOffset = dir->imageOffset,
        .imageExtent = dir->extent
    };

    vkCmdCopyBufferToImage(cmd_block.buffer, dir->buffer, dir->image, dir->destLayout, 1, &region);

    finite_render_finish_onshot_command(render, cmd_block);
}