#ifndef __RENDER_IMAGE_H__
#define __RENDER_IMAGE_H__
#include "render-core.h"

typedef struct FiniteRenderImageInfo FiniteRenderImageInfo;
typedef struct FiniteRenderImage FiniteRenderImage;
typedef struct FiniteRenderTextureInfo FiniteRenderTextureInfo;
typedef struct FiniteRenderImageBarrierInfo FiniteRenderImageBarrierInfo;
typedef struct FiniteRenderPipelineDirections FiniteRenderPipelineDirections;
typedef struct FiniteRenderImageCopyDirections FiniteRenderImageCopyDirections;
typedef struct FiniteRenderImageViewInfo FiniteRenderImageViewInfo;
typedef struct FiniteRenderTextureSamplerInfo FiniteRenderTextureSamplerInfo;

typedef unsigned char stbi_uc;

struct FiniteRenderImage {
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;
};

struct FiniteRenderImageInfo {
    const void *next;
    VkImageCreateFlags flags;
    VkImageType imageType;
    VkFormat format;
    VkExtent3D extent;
    uint32_t _mipLevels;
    uint32_t _layers;
    VkSampleCountFlagBits _samples;
    VkImageTiling tiling;
    VkImageUsageFlags useFlags;
    VkSharingMode sharing;
    uint32_t _fIndex;
    const uint32_t *fIndex;
    VkImageLayout layout;
};

struct FiniteRenderTextureInfo {
    int width;
    int height;
    int channels;
    VkDeviceSize size;
    stbi_uc *pixels;
};

struct FiniteRenderImageBarrierInfo {
    const void *next;
    VkAccessFlags srcFlags;
    VkAccessFlags destFlags;
    VkImageLayout old;
    VkImageLayout new;
    uint32_t srcfIndex;
    uint32_t destfIndex;
    VkImage image;
    VkImageSubresourceRange subRange;    
};

struct FiniteRenderPipelineDirections {
    VkPipelineStageFlags srcFlags;
    VkPipelineStageFlags destFlags;
    VkDependencyFlags depFlags;
};

struct FiniteRenderImageCopyDirections {
    VkDeviceSize offset;
    uint32_t rowLength;
    uint32_t height;
    VkImageSubresourceLayers subLayers;
    VkOffset3D imageOffset;
    VkExtent3D extent;
    VkBuffer buffer;
    VkImage image;
    VkImageLayout destLayout;
};

struct FiniteRenderImageViewInfo {
    const void *next;
    VkImageViewCreateFlags flags;
    VkImage image;
    VkImageViewType type;
    VkFormat format;
    VkComponentMapping components;
    VkImageSubresourceRange subRange;
};

struct FiniteRenderTextureSamplerInfo {
    const void *next;
    VkSamplerCreateFlags flags;
    VkFilter magFilter;
    VkFilter minFilter;
    VkSamplerMipmapMode mipmapMode;
    VkSamplerAddressMode addressModeU;
    VkSamplerAddressMode addressModeV;
    VkSamplerAddressMode addressModeW;
    float mipLodBias;
    bool anisotropyEnable;
    float maxAnisotropy;
    bool compareEnable;
    VkCompareOp compareOp;
    float minLod;
    float maxLod;
    VkBorderColor borderColor;
    bool unnormalizedCoordinates;
};

#define finite_render_create_texture(file, info, forceAlpha) finite_render_create_texture_debug(__FILE__, __func__, __LINE__, file, info, forceAlpha)
void finite_render_create_texture_debug(const char *rfile, const char *func, int line, const char *file, FiniteRenderTextureInfo *info, bool forceAlpha);

void finite_render_destroy_pixels(FiniteRenderTextureInfo *image);
void finite_render_cleanup_textures(FiniteRender *render, FiniteRenderImage *imgs, uint32_t _imgs);

#define finite_render_create_image(render, info, mem_info) finite_render_create_image_debug(__FILE__, __func__, __LINE__, render, info, mem_info)
FiniteRenderImage *finite_render_create_image_debug(const char *file, const char *func, int line, FiniteRender *render, FiniteRenderImageInfo *info, FiniteRenderMemAllocInfo *mem_info);
void finite_render_create_view(FiniteRender *render, FiniteRenderImage *img, FiniteRenderImageViewInfo *info);
void finite_render_create_sampler(FiniteRender *render, FiniteRenderImage *img, FiniteRenderTextureSamplerInfo *info);

#define finite_render_transition_image_layout(render, info, format, dir) finite_render_transition_image_layout_debug(__FILE__, __func__, __LINE__, render, info, format, dir)
void finite_render_transition_image_layout_debug(const char *file, const char *func, int line, FiniteRender *render, FiniteRenderImageBarrierInfo *info, VkFormat format, FiniteRenderPipelineDirections *dir);

void finite_render_copy_buffer_to_image(FiniteRender *render, FiniteRenderImageCopyDirections *dir);

#define finite_render_generate_mipmaps(render, info) finite_render_generate_mipmaps_debug(__FILE__, __func__, __LINE__, render, info)
void finite_render_generate_mipmaps_debug(const char *file, const char *func, int line, FiniteRender *render, FiniteRenderImageBarrierInfo *info);

#endif