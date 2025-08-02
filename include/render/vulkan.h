#ifndef __VULKAN_H__
#define __VULKAN_H__
#include "render-core.h"

#define finite_render_init(shell, extensions, layers, _exts, _layers) finite_render_init_debug(__FILE__, __func__, __LINE__, shell, extensions, layers, _exts, _layers)
FiniteRender *finite_render_init(const char *file, const char *func, int line, FiniteShell *shell, char **extensions, char **layers, uint32_t _exts, uint32_t _layers);

#define finite_render_create_physical_device(render) finite_render_create_physical_device_debug(__FILE__, __func__, __LINE__, render)
void finite_render_create_physical_device_debug(const char *file, const char *func, int line, FiniteRender *render);

#define finite_render_create_device(render, fIndex, uniqueQueueFamilies, device_extensions, _exts) finite_render_create_device_debug(__FILE__, __func__, __LINE__, render, fIndex, uniqueQueueFamilies, device_extensions, _exts)
void finite_render_create_device_debug(const char *file, const char *func, int line, FiniteRender *render, FiniteRenderQueueFamilies fIndex, uint32_t *uniqueQueueFamilies, char **device_extentsions, uint32_t _ext);

#define finite_render_create_swapchain(render, info) finite_render_create_swapchain_debug(__FILE__, __func__, __LINE__, render, info)
void finite_render_create_swapchain_debug(const char *file, const char *func, int line, FiniteRender *render, FiniteRenderSwapchainInfo info);

#define finite_render_create_swapchain_images(render) finite_render_create_swapchain_images_debug(__FILE__, __func__, __LINE__, render)
void finite_render_create_swapchain_images_debug(const char *file, const char *func, int line, FiniteRender *render);

#define finite_render_create_example_render_pass(render) finite_render_create_example_render_pass_debug(__FILE__, __func__, __LINE__, render)
void finite_render_create_example_render_pass_debug(const char *file, const char *func, int line, FiniteRender *render);

#define finite_render_create_render_pass(render, att_desc_info, ref_info, subpass_desc_info, subpass_dep_info, info) finite_render_create_render_pass_debug(__FILE__, __func__, __LINE__, render, att_desc_info, ref_info, subpass_desc_info, subpass_dep_info, info)
void finite_render_create_render_pass_debug(const char *file, const char *func, int line, FiniteRender *render, FiniteRenderAttachmentDescriptionInfo **att_desc_info, FiniteRenderAttachmentRefInfo **ref_info, FiniteRenderSubpassDescriptionInfo **subpass_desc_info, FiniteRenderSubpassDependencyInfo **subpass_dep_info, FiniteRenderRenderPassInfo *info);

#define finite_render_create_framebuffers(render, info) finite_render_create_framebuffers_debug(__FILE__, __func__, __LINE__, render, info)
void finite_render_create_framebuffers_debug(const char *file, const char *func, int line, FiniteRender *render, FiniteRenderFramebufferInfo *info);

#define finite_render_create_pipeline_layout(render, layoutInfo) finite_render_create_pipeline_layout_debug(__FILE__, __func__, __LINE__, render, layoutInfo)
void finite_render_create_pipeline_layout_debug(const char *file, const char *func, int line, FiniteRender *render, FiniteRenderPipelineLayoutInfo *layoutInfo);

#define finite_render_add_shader_stage(render, stage) finite_render_add_shader_stage_debug(__FILE__, __func__, __LINE__, render, stage)
bool finite_render_add_shader_stage_debug(__FILE__, __func__, __LINE__, FiniteRender *render, FiniteRenderShaderStageInfo *stage);

#define finite_render_create_vertex_input(render, vertex) finite_render_create_vertex_input_debug(render, vertex)
VkPipelineVertexInputStateCreateInfo finite_render_create_vertex_input_debug(FiniteRender *render, FiniteRenderVertexInputInfo *vertex);

#define finite_render_create_assembly_state(render, assemble) finite_render_create_assembly_state_debug(__FILE__, __func__, __LINE__, render, assemble)
VkPipelineInputAssemblyStateCreateInfo finite_render_create_assembly_state_debug(const char *file, const char *func, int line, FiniteRender *render, FiniteRenderAssemblyInfo *assemble);

#define finite_render_create_viewport_state(render, state) finite_render_create_viewport_state_debug(__FILE__, __func__, __LINE__, render, state)
VkPipelineViewportStateCreateInfo finite_render_create_viewport_state_debug(const char *file, const char *func, int line, FiniteRender *render, FiniteRenderViewportState *state);

#define finite_render_create_raster_info(render, state) finite_render_create_raster_info_debug(__FILE__, __func__, __LINE__, render, state)
VkPipelineRasterizationStateCreateInfo finite_render_create_raster_info_debug(const char *file, const char *func, int line, FiniteRender *render, FiniteRenderRasterState *state);

#define finite_render_create_multisample_info(render, info) finite_render_create_multisample_info_debug(__FILE__, __func__, __LINE__, render, info)
VkPipelineMultisampleStateCreateInfo finite_render_create_multisample_info_debug(FiniteRender *render, FiniteRenderMultisampleStateInfo *info);

#define finite_render_create_color_blend_attachment(att) finite_render_create_color_blend_attachment_debug(__FILE__, __func__, __LINE__, att)
VkPipelineColorBlendAttachmentState finite_render_create_color_blend_attachment_debug(const char *file, const char *func, int line, FiniteRenderColorAttachmentInfo *att);

#define finite_render_create_color_blend_state(render, att, blend) finite_render_create_color_blend_state_debug(__FILE__, __func__, __LINE__, render, att, blend)
VkPipelineColorBlendStateCreateInfo finite_render_create_color_blend_state_debug(__FILE__, __func__, __LINE__, FiniteRender *render, VkPipelineColorBlendAttachmentState *att, FiniteRenderColorBlendInfo *blend);

#define finite_render_create_graphics_pipeline(render, flags, vertex, assemble, tees, port, raster, sample, blend, dyna) finite_render_create_graphics_pipeline_debug(__FILE__, __func__, __LINE__, render, flags, vertex, assemble, tees, port, raster, sample, blend, dyna)
bool finite_render_create_graphics_pipeline_debug(const char *file, const char *func, int line, FiniteRender *render, VkPipelineCreateFlags flags, VkPipelineVertexInputStateCreateInfo *vertex, VkPipelineInputAssemblyStateCreateInfo *assemble, VkPipelineTessellationStateCreateInfo *tess, VkPipelineViewportStateCreateInfo *port, VkPipelineRasterizationStateCreateInfo *raster, VkPipelineMultisampleStateCreateInfo *sample, VkPipelineColorBlendStateCreateInfo *blend, VkPipelineDynamicStateCreateInfo *dyna);

#define finite_render_create_command_buffer(render, autocreate, isPrimary, _buffs) finite_render_create_command_buffer_debug(__FILE__, __func__, __LINE__, render, autocreate, isPrimary, _buffs)
bool finite_render_create_command_buffer_debug(const char *file, const char *func, int line, FiniteRender *render, bool autocreate, bool isPrimary, uint32_t _buffs);

#define finite_render_create_semaphore(render) finite_render_create_semaphore_debug(__FILE__, __func__, __LINE__, render)
bool finite_render_create_semaphore_debug(const char *file, const char *func, int line, FiniteRender *render);

#define finite_render_create_fence(render, initialState) finite_render_create_fence_debug(__FILE__, __func__, __LINE__, render, initialState)
bool finite_render_create_fence_debug(const char *file, const char *func, int line, FiniteRender *render, VkFenceCreateFlags initialState);

#define finite_render_submit_frame(render, info, fenceId, safeExit) finite_render_submit_frame_debug(__FILE__, __func__, __LINE__, render, info, fenceId, safeExit)
bool finite_render_submit_frame_debug(const char *file, const char *func, int line, FiniteRender *render, FiniteRenderSubmitInfo *info, uint32_t fenceId, bool safeExit);

#define finite_render_present_frame(render, info, safeExit) finite_render_present_frame_debug(__FILE__, __func__, __LINE__, render, info, safeExit)
bool finite_render_present_frame_debug(FiniteRender *render, FiniteRenderPresentInfo *info, bool safeExit);

bool finite_render_create_vertex_buffer(FiniteRender *render, FiniteRenderBufferInfo *info, FiniteRenderMemAllocInfo *mem_info, uint64_t vertexSize, FiniteRenderReturnBuffer *rtrn);
bool finite_render_alloc_buffer_memory(FiniteRender *render, FiniteRenderMemAllocInfo *info, VkDeviceSize offset);
bool finite_render_create_descriptor_layout(FiniteRender *render, FiniteRenderDescriptorSetLayout **info, uint32_t layouts);
bool finite_render_create_uniform_buffer(FiniteRender *render, FiniteRenderBufferInfo *info, FiniteRenderMemAllocInfo *mem_info);
bool finite_render_create_descriptor_pool(FiniteRender *render, FiniteRenderDescriptorPoolInfo **info, bool autoCreate, uint32_t _infos);
bool finite_render_write_to_descriptor(FiniteRender *render, FiniteRenderWriteSetInfo **info, FiniteRenderDescriptorInfo *desc_info, uint32_t _infos);
bool finite_render_create_generic_buffer(FiniteRender *render, FiniteRenderBufferInfo *info, FiniteRenderMemAllocInfo *mem_info, uint64_t vertexSize, FiniteRenderReturnBuffer *rtrn);
#endif