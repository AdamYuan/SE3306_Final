#include "MBTilePass.hpp"

namespace rg {

void MBTileMaxPass::CreatePipeline() {
	const auto &device = GetRenderGraphPtr()->GetDevicePtr();

	auto pipeline_layout = myvk::PipelineLayout::Create(device, {GetVkDescriptorSetLayout()}, {});
	constexpr uint32_t kCompSpv[] = {
#include <shader/mb_tile_max.comp.u32>
	};

	std::shared_ptr<myvk::ShaderModule> shader_module;
	shader_module = myvk::ShaderModule::Create(device, kCompSpv, sizeof(kCompSpv));

	uint32_t spec_data[] = {m_tile_size, m_subgroup_size, m_shared_size};
	VkSpecializationMapEntry spec_entries[] = {
	    {.constantID = 0, .offset = 0, .size = sizeof(uint32_t)},
	    {.constantID = 1, .offset = sizeof(uint32_t), .size = sizeof(uint32_t)},
	    {.constantID = 2, .offset = 2 * sizeof(uint32_t), .size = sizeof(uint32_t)},
	};
	VkSpecializationInfo spec_info = {3, spec_entries, sizeof(spec_data), spec_data};

	VkPipelineShaderStageCreateInfo shader_stage = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
	// if (m_subgroup_size > 1)
	// 	shader_stage.flags = VK_PIPELINE_SHADER_STAGE_CREATE_REQUIRE_FULL_SUBGROUPS_BIT;
	shader_stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shader_stage.module = shader_module->GetHandle();
	shader_stage.pName = "main";
	shader_stage.pSpecializationInfo = &spec_info;

	VkComputePipelineCreateInfo create_info = {VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
	create_info.layout = pipeline_layout->GetHandle();
	create_info.stage = shader_stage;

	m_pipeline = myvk::ComputePipeline::Create(pipeline_layout, create_info);
}

void MBTileMaxPass::CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const {
	command_buffer->CmdBindPipeline(m_pipeline);
	command_buffer->CmdBindDescriptorSets({GetVkDescriptorSet()}, m_pipeline);
	auto canvas_size = GetRenderGraphPtr()->GetCanvasSize();
	command_buffer->CmdDispatch(div_ceil(canvas_size.width, m_tile_size), div_ceil(canvas_size.height, m_tile_size), 1);
}

void MBTileNeiPass::CreatePipeline() {
	auto pipeline_layout =
	    myvk::PipelineLayout::Create(GetRenderGraphPtr()->GetDevicePtr(), {GetVkDescriptorSetLayout()}, {});

	const auto &device = GetRenderGraphPtr()->GetDevicePtr();

	constexpr uint32_t kVertSpv[] = {
#include <shader/quad.vert.u32>
	};
	constexpr uint32_t kFragSpv[] = {
#include <shader/mb_tile_nei.frag.u32>
	};

	std::shared_ptr<myvk::ShaderModule> vert_shader_module, frag_shader_module;
	vert_shader_module = myvk::ShaderModule::Create(device, kVertSpv, sizeof(kVertSpv));
	frag_shader_module = myvk::ShaderModule::Create(device, kFragSpv, sizeof(kFragSpv));

	std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {
	    vert_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
	    frag_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)};

	myvk::GraphicsPipelineState pipeline_state = {};
	auto extent = GetRenderGraphPtr()->GetCanvasSize();
	extent = VkExtent2D{div_ceil(extent.width, m_tile_size), div_ceil(extent.height, m_tile_size)};
	pipeline_state.m_viewport_state.Enable(std::vector<VkViewport>{{0, 0, (float)extent.width, (float)extent.height}},
	                                       std::vector<VkRect2D>{{{0, 0}, extent}});
	pipeline_state.m_vertex_input_state.Enable();
	pipeline_state.m_input_assembly_state.Enable(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipeline_state.m_rasterization_state.Initialize(VK_POLYGON_MODE_FILL, VK_FRONT_FACE_COUNTER_CLOCKWISE,
	                                                VK_CULL_MODE_FRONT_BIT);
	pipeline_state.m_multisample_state.Enable(VK_SAMPLE_COUNT_1_BIT);
	pipeline_state.m_color_blend_state.Enable(1, VK_FALSE);

	m_pipeline =
	    myvk::GraphicsPipeline::Create(pipeline_layout, GetVkRenderPass(), shader_stages, pipeline_state, GetSubpass());
}
void MBTileNeiPass::CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const {
	command_buffer->CmdBindPipeline(m_pipeline);
	command_buffer->CmdBindDescriptorSets({GetVkDescriptorSet()}, m_pipeline);
	command_buffer->CmdDraw(3, 1, 0, 0);
}

} // namespace rg
