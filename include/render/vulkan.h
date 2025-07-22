#ifndef __VULKAN_H__
#define __VULKAN_H__
#include "render-core.h"

FiniteRender *finite_render_init(FiniteShell *shell, char **extensions, char **layers, uint32_t _exts, uint32_t _layers);
void finite_render_create_physical_device(FiniteRender *render);
void finite_render_create_device(FiniteRender *render, FiniteRenderQueueFamilies fIndex, uint32_t *uniqueQueueFamilies, char **device_extentsions, uint32_t _ext);
void finite_render_create_swapchain(FiniteRender *render, FiniteRenderSwapchainInfo info);
void finite_render_create_swapchain_images(FiniteRender *render);
void finite_render_create_example_render_pass(FiniteRender *render);
void finite_render_create_framebuffers(FiniteRender *render);
void finite_render_create_pipeline_layout( FiniteRender *render, FiniteRenderPipelineLayoutInfo *layoutInfo);
bool finite_render_add_shader_stage(FiniteRender *render, FiniteRenderShaderStageInfo *stage);
VkPipelineVertexInputStateCreateInfo finite_render_create_vertex_input(FiniteRender *render, FiniteRenderVertexInputInfo *vertex);
VkPipelineInputAssemblyStateCreateInfo finite_render_create_assembly_state(FiniteRender *render, FiniteRenderAssemblyInfo *assemble);
VkPipelineViewportStateCreateInfo finite_render_create_viewport_state(FiniteRender *render, FiniteRenderViewportState *state);
VkPipelineRasterizationStateCreateInfo finite_render_create_raster_info(FiniteRender *render, FiniteRenderRasterState *state);
VkPipelineMultisampleStateCreateInfo finite_render_create_multisample_info(FiniteRender *render, FiniteRenderMultisampleStateInfo *info);
VkPipelineColorBlendAttachmentState finite_render_create_color_blend_attachment(FiniteRenderColorAttachmentInfo *att);
VkPipelineColorBlendStateCreateInfo finite_render_create_color_blend_state(FiniteRender *render, VkPipelineColorBlendAttachmentState *att, FiniteRenderColorBlendInfo *blend);
bool finite_render_create_graphics_pipeline(FiniteRender *render, VkPipelineCreateFlags flags, VkPipelineVertexInputStateCreateInfo *vertex, VkPipelineInputAssemblyStateCreateInfo *assemble, VkPipelineTessellationStateCreateInfo *tess, VkPipelineViewportStateCreateInfo *port, VkPipelineRasterizationStateCreateInfo *raster, VkPipelineMultisampleStateCreateInfo *sample, VkPipelineColorBlendStateCreateInfo *blend, VkPipelineDynamicStateCreateInfo *dyna);
bool finite_render_create_command_buffer(FiniteRender *render, bool autocreate, bool isPrimary, uint32_t _buffs);
bool finite_render_create_semaphore(FiniteRender *render);
bool finite_render_create_fence(FiniteRender *render, VkFenceCreateFlags initialState);
bool finite_render_submit_frame(FiniteRender *render, FiniteRenderSubmitInfo *info, uint32_t fenceId, bool safeExit);
bool finite_render_present_frame(FiniteRender *render, FiniteRenderPresentInfo *info, bool safeExit);
bool finite_render_create_vertex_buffer(FiniteRender *render, FiniteRenderBufferInfo *info, FiniteRenderMemAllocInfo *mem_info, uint64_t vertexSize, FiniteRenderReturnBuffer *rtrn);
bool finite_render_alloc_buffer_memory(FiniteRender *render, FiniteRenderMemAllocInfo *info, VkDeviceSize offset);
bool finite_render_create_descriptor_layout(FiniteRender *render, FiniteRenderDescriptorSetLayout **info, uint32_t layouts);
bool finite_render_create_uniform_buffer(FiniteRender *render, FiniteRenderBufferInfo *info, FiniteRenderMemAllocInfo *mem_info);
bool finite_render_create_descriptor_pool(FiniteRender *render, FiniteRenderDescriptorPoolInfo **info, bool autoCreate, uint32_t _infos);
bool finite_render_write_to_descriptor(FiniteRender *render, FiniteRenderWriteSetInfo **info, FiniteRenderDescriptorInfo *desc_info, uint32_t _infos);
bool finite_render_create_generic_buffer(FiniteRender *render, FiniteRenderBufferInfo *info, FiniteRenderMemAllocInfo *mem_info, uint64_t vertexSize, FiniteRenderReturnBuffer *rtrn);
#endif