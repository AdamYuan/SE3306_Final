#include "GBufferPass.hpp"

namespace rg {
void GBufferPass::CreatePipeline() {
	const auto &device = GetRenderGraphPtr()->GetDevicePtr();

	auto pipeline_layout =
		myvk::PipelineLayout::Create(device, {m_ani_instance.GetDescriptorSet()->GetDescriptorSetLayoutPtr()},
		                             {{VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4) + sizeof(glm::vec2)}});

	constexpr uint32_t kVertSpv[] = {
#include <shader/gbuffer.vert.u32>
	};
	constexpr uint32_t kFragSpv[] = {
#include <shader/gbuffer.frag.u32>
	};

	std::shared_ptr<myvk::ShaderModule> vert_shader_module, frag_shader_module;
	vert_shader_module = myvk::ShaderModule::Create(device, kVertSpv, sizeof(kVertSpv));
	frag_shader_module = myvk::ShaderModule::Create(device, kFragSpv, sizeof(kFragSpv));

	std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {
		vert_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
		frag_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)};

	myvk::GraphicsPipelineState pipeline_state = {};
	pipeline_state.m_vertex_input_state.Enable(GPUMeshInstance::GetVertexInputBindings(),
	                                           GPUMeshInstance::GetVertexInputAttributes());
	pipeline_state.m_input_assembly_state.Enable(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipeline_state.m_rasterization_state.Initialize(VK_POLYGON_MODE_FILL, VK_FRONT_FACE_COUNTER_CLOCKWISE,
	                                                VK_CULL_MODE_BACK_BIT);
	pipeline_state.m_depth_stencil_state.Enable(VK_TRUE, VK_TRUE);
	pipeline_state.m_multisample_state.Enable(VK_SAMPLE_COUNT_1_BIT);
	pipeline_state.m_color_blend_state.Enable(3, VK_FALSE);
	auto extent = GetRenderGraphPtr()->GetCanvasSize();
	pipeline_state.m_viewport_state.Enable(
		std::vector<VkViewport>{{0, 0, (float)extent.width, (float)extent.height, 0.0f, 1.0f}},
		std::vector<VkRect2D>{{{0, 0}, extent}});

	m_pipeline =
		myvk::GraphicsPipeline::Create(pipeline_layout, GetVkRenderPass(), shader_stages, pipeline_state, GetSubpass());
}

void GBufferPass::CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const {
	command_buffer->CmdBindPipeline(m_pipeline);
	command_buffer->CmdBindDescriptorSets({m_ani_instance.GetDescriptorSet()}, m_pipeline);
	float pc_data[16 + 2];
	*(glm::mat4 *)pc_data = Animation::GetCameraViewProj();
	*(glm::vec2 *)(pc_data + 16) = {}; // jitter
	command_buffer->CmdPushConstants(m_pipeline->GetPipelineLayoutPtr(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pc_data),
	                                 pc_data);
	m_ani_instance.CmdDraw(command_buffer, 0, 0, 0, 0, 0);
}
}