#include "VoxelizePass.hpp"

namespace rg {

void VoxelDrawPass::CreatePipeline() {
	const auto &device = GetRenderGraphPtr()->GetDevicePtr();

	auto pipeline_layout = myvk::PipelineLayout::Create(
	    device, {GetVkDescriptorSetLayout(), m_ani_instance.GetDescriptorSet()->GetDescriptorSetLayoutPtr()},
	    {{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4)}});

	constexpr uint32_t kVertSpv[] = {
#include <shader/voxelize.vert.u32>
	};
	constexpr uint32_t kGeomSpv[] = {
#include <shader/voxelize.geom.u32>
	};
	constexpr uint32_t kFragSpv[] = {
#include <shader/voxelize.frag.u32>
	};

	std::shared_ptr<myvk::ShaderModule> vert_shader_module, geom_shader_module, frag_shader_module;
	vert_shader_module = myvk::ShaderModule::Create(device, kVertSpv, sizeof(kVertSpv));
	geom_shader_module = myvk::ShaderModule::Create(device, kGeomSpv, sizeof(kGeomSpv));
	frag_shader_module = myvk::ShaderModule::Create(device, kFragSpv, sizeof(kFragSpv));

	std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {
	    vert_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
	    geom_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_GEOMETRY_BIT),
	    frag_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)};

	myvk::GraphicsPipelineState pipeline_state = {};
	pipeline_state.m_vertex_input_state.Enable(GPUMeshInstance::GetVertexInputBindings(),
	                                           GPUMeshInstance::GetVertexInputAttributes());
	pipeline_state.m_input_assembly_state.Enable(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipeline_state.m_rasterization_state.Initialize(VK_POLYGON_MODE_FILL, VK_FRONT_FACE_COUNTER_CLOCKWISE,
	                                                VK_CULL_MODE_NONE);
	pipeline_state.m_multisample_state.Enable(VK_SAMPLE_COUNT_1_BIT);
	pipeline_state.m_viewport_state.Enable(
	    std::vector<VkViewport>{{0, 0, (float)m_resolution, (float)m_resolution, 0.0f, 0.0f}},
	    std::vector<VkRect2D>{{{0, 0}, VkExtent2D{m_resolution, m_resolution}}});

	m_pipeline =
	    myvk::GraphicsPipeline::Create(pipeline_layout, GetVkRenderPass(), shader_stages, pipeline_state, GetSubpass());
}

void VoxelDrawPass::CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const {
	command_buffer->CmdBindPipeline(m_pipeline);
	command_buffer->CmdBindDescriptorSets({GetVkDescriptorSet(), m_ani_instance.GetDescriptorSet()}, m_pipeline);
	float pc_data[16];
	*(glm::mat4 *)pc_data = Animation::GetShadowViewProj();
	command_buffer->CmdPushConstants(m_pipeline->GetPipelineLayoutPtr(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pc_data),
	                                 pc_data);
	m_ani_instance.CmdDraw(command_buffer, {.opt_cornell_lod = 1,
	                                        .opt_tumbler_lod = 1,
	                                        // .opt_marble_lod = 1,
	                                        .opt_fireball_lod = 1,
	                                        .opt_particle_lod = 1});
}

} // namespace rg
