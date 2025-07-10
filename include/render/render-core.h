#ifndef __RENDER_CORE_H__
#define __RENDER_CORE_H__

#define VK_USE_PLATFORM_WAYLAND_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>
#include <finite/draw.h>

typedef struct FiniteRenderSwapchainInfo FiniteRenderSwapchainInfo;
typedef struct FiniteRender FiniteRender;
typedef struct FiniteRenderQueueFamilies FiniteRenderQueueFamilies;
typedef struct FiniteRenderPipelineLayoutInfo FiniteRenderPipelineLayoutInfo;
typedef struct FiniteRenderShaderStages FiniteRenderShaderStages;
typedef struct FiniteRenderShaderStageInfo FiniteRenderShaderStageInfo;
typedef struct FiniteRenderVertexInputInfo FiniteRenderVertexInputInfo;
typedef struct FiniteRenderAssemblyInfo FiniteRenderAssemblyInfo;
typedef struct FiniteRenderViewportState FiniteRenderViewportState;
typedef struct FiniteRenderRasterState FiniteRenderRasterState;
typedef struct FiniteRenderMultisampleStateInfo FiniteRenderMultisampleStateInfo;
typedef struct FiniteRenderColorAttachmentInfo FiniteRenderColorAttachmentInfo;
typedef struct FiniteRenderColorBlendInfo FiniteRenderColorBlendInfo;
typedef struct FiniteRenderVertexBufferInfo FiniteRenderVertexBufferInfo;
typedef struct FiniteRenderMemAllocInfo FiniteRenderMemAllocInfo;
typedef struct FiniteRenderSubmitInfo FiniteRenderSubmitInfo;
typedef struct FiniteRenderPresentInfo FiniteRenderPresentInfo;
typedef enum   FiniteShaderType FiniteShaderType;

enum FiniteShaderType {
    FINITE_SHADER_TYPE_VERTEX,
    FINITE_SHADER_TYPE_FRAGMENT
};

struct FiniteRenderQueueFamilies {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    uint32_t _unique;
};

struct FiniteRenderSwapchainInfo {
    VkSurfaceCapabilitiesKHR caps;
    uint32_t _forms;
    VkSurfaceFormatKHR *forms;
    uint32_t _modes;
    VkPresentModeKHR *modes;
};

struct FiniteRenderShaderStages {
    uint32_t _stages;
    VkPipelineShaderStageCreateInfo *infos;
};

struct FiniteRender {
    FiniteShell *shell;
    char **required_layers; // array of extension layers (usually just validation which is on by default)
    char **required_extensions;  // required instance level extensions
    char **required_deviceExtensions;
    uint32_t _images;
    uint32_t _layers;
    uint32_t _exts;
    uint32_t _devExts;
    uint32_t _modules;
    uint32_t _signals;
    uint32_t _fences;

    FiniteRenderShaderStages stages;
    
    VkInstance vk_instance;
    VkPhysicalDevice vk_pDevice;
    VkDevice vk_device;
    VkSurfaceKHR vk_surface;
    VkQueue vk_graphicsQueue;
    VkQueue vk_presentQueue;

    VkSwapchainKHR vk_swapchain;

    VkImage *vk_image;
    VkSurfaceFormatKHR vk_imageForm;
    VkPresentModeKHR mode;
    VkExtent2D vk_extent;
    VkImageView *vk_view;
    VkPipelineLayout vk_layout;
    VkRenderPass vk_renderPass;
    VkPipeline vk_pipeline;
    VkShaderModule *modules; // array of shader modules
    VkFramebuffer *vk_frameBufs;
    VkCommandPool vk_pool;
    VkCommandBuffer vk_buffer;

    VkBuffer vk_vertexBuf;
    VkDeviceMemory vk_memory;

    VkSemaphore *signals;
    VkFence *fences;
};

struct FiniteRenderPipelineLayoutInfo {
    VkPipelineLayoutCreateFlags flags;
    uint32_t _setConsts;
    VkDescriptorSetLayout *setConsts;
    uint32_t _pushRange;
    VkPushConstantRange *pushRange;
};

struct FiniteRenderShaderStageInfo {
    const void* next;
    VkPipelineShaderStageCreateFlags flags;
    FiniteShaderType stage;
    VkShaderModule shader;
    const char* name;
    const VkSpecializationInfo* specializationInfo;    
};

struct FiniteRenderVertexInputInfo {
    const void* next;
    VkPipelineVertexInputStateCreateFlags flags;
    uint32_t _vertexBindings;
    const VkVertexInputBindingDescription* vertexBindingDescriptions;
    uint32_t _vertexAtributes;
    const VkVertexInputAttributeDescription* vertexAttributeDescriptions;
};

struct FiniteRenderAssemblyInfo {
    const void* next;
    VkPipelineInputAssemblyStateCreateFlags flags;
    VkPrimitiveTopology topology;
    bool primitiveRestartEnable;
};

struct FiniteRenderViewportState {
    const void* next;
    VkPipelineViewportStateCreateFlags flags;
    uint32_t _viewports;
    const VkViewport *viewports;
    uint32_t _scissors;
    const VkRect2D *scissors;
};

struct FiniteRenderRasterState {
    const void* next;
    VkPipelineRasterizationStateCreateFlags flags;
    bool depthClampEnable;
    bool rasterizerDiscardEnable;
    VkPolygonMode polygonMode;
    VkCullModeFlags cullMode;
    VkFrontFace frontFace;
    bool depthBiasEnable;
    float depthBiasConstantFactor;
    float depthBiasClamp;
    float depthBiasSlopeFactor;
    float lineWidth;
};

struct FiniteRenderMultisampleStateInfo {
    const void* next;
    VkPipelineMultisampleStateCreateFlags flags;
    VkSampleCountFlagBits rasterizationSamples;
    bool sampleShadingEnable;
    float minSampleShading;
    const VkSampleMask* sampleMask;
    bool alphaToCoverageEnable;
    bool alphaToOneEnable;
};

struct FiniteRenderColorAttachmentInfo {
    bool blendEnable;
    VkBlendFactor srcColorBlendFactor;
    VkBlendFactor dstColorBlendFactor;
    VkBlendOp colorBlendOp;
    VkBlendFactor srcAlphaBlendFactor;
    VkBlendFactor dstAlphaBlendFactor;
    VkBlendOp alphaBlendOp;
    VkColorComponentFlags colorWriteMask;
};

struct FiniteRenderColorBlendInfo {
    const void* next;
    VkPipelineColorBlendStateCreateFlags flags;
    bool logicOpEnable;
    VkLogicOp logicOp;
    uint32_t _attachments;
    const VkPipelineColorBlendAttachmentState *attachments;
    float blendConstants[4];
};

struct FiniteRenderVertexBufferInfo {
    const void *next;
    VkBufferCreateFlags flags;
    VkDeviceSize size;
    VkBufferUsageFlags useFlags;
    VkSharingMode sharing;
    uint32_t _fIndex;
    uint32_t *fIndex;
};

struct FiniteRenderMemAllocInfo {
    const void *next;
    VkDeviceSize size;
    uint32_t type;
};

struct FiniteRenderSubmitInfo {
    const void *next;
    uint32_t _waitSemaphores;
    const VkSemaphore *waitSemaphores;
    const VkPipelineStageFlags *waitDstStageMask;
    uint32_t _commandBuffs;
    const VkCommandBuffer *commandBuffs;
    uint32_t _signalSemaphores;
    const VkSemaphore *signalSemaphores;
};

struct FiniteRenderPresentInfo {
    const void *next;
    uint32_t _waitSemaphores;
    const VkSemaphore *waitSemaphores;
    uint32_t _swapchains;
    const VkSwapchainKHR *swapchains;
    const uint32_t *imageIndices;
    VkResult *results;
};

FiniteRenderQueueFamilies finite_render_find_queue_families(VkPhysicalDevice pDevice, VkSurfaceKHR vk_surface);
bool finite_render_check_extensions(FiniteRender *render, VkPhysicalDevice pDevice);
FiniteRenderSwapchainInfo finite_render_get_swapchain_info(FiniteRender *render, VkPhysicalDevice pDevice);
VkSurfaceFormatKHR finite_render_get_best_format(FiniteRender *render, VkSurfaceFormatKHR *forms, uint32_t _forms);
VkPresentModeKHR finite_render_get_best_present_mode(FiniteRender *render, VkPresentModeKHR *modes, uint32_t _modes);
VkExtent2D finite_render_get_best_extent(FiniteRender *render, VkSurfaceCapabilitiesKHR *caps, FiniteShell *shell);
bool finite_render_check_device(FiniteRender *render, VkPhysicalDevice pDevice);
void finite_render_record_command_buffer(FiniteRender *render, uint32_t index, VkDeviceSize offset, uint32_t _verts);
bool finite_render_get_shader_module(FiniteRender *render, char *code, uint32_t size);
void finite_render_cleanup(FiniteRender *render);
char *finite_render_get_shader_code(const char *fileName, uint32_t *pShaderSize);
uint32_t finite_render_get_memory_format(FiniteRender *render, uint32_t filter, VkMemoryPropertyFlags props);

#endif