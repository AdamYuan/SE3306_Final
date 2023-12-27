#include "ShadowMapPass.hpp"

namespace rg {

void ShadowMapPass::CreatePipeline() {
	const auto &device = GetRenderGraphPtr()->GetDevicePtr();

	auto pipeline_layout = myvk::PipelineLayout::Create(device, {}, {});

	constexpr uint32_t kVertSpv[] = {
#include <shader/shadow.vert.u32>
	};
	constexpr uint32_t kFragSpv[] = {
#include <shader/shadow.frag.u32>
	};

	std::shared_ptr<myvk::ShaderModule> vert_shader_module, frag_shader_module;
	vert_shader_module = myvk::ShaderModule::Create(device, kVertSpv, sizeof(kVertSpv));
	frag_shader_module = myvk::ShaderModule::Create(device, kFragSpv, sizeof(kFragSpv));

	auto shadow_view_proj = Animation::GetShadowViewProj();
	for (int i = 0; i < 16; ++i)
		vert_shader_module->AddSpecialization(i, shadow_view_proj[i / 4][i % 4]);

	std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {
	    vert_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
	    frag_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)};

	myvk::GraphicsPipelineState pipeline_state = {};
	pipeline_state.m_vertex_input_state.Enable(GPUMeshInstance::GetVertexInputBindings(),
	                                           GPUMeshInstance::GetVertexInputAttributes());
	pipeline_state.m_input_assembly_state.Enable(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipeline_state.m_rasterization_state.Initialize(VK_POLYGON_MODE_FILL, VK_FRONT_FACE_COUNTER_CLOCKWISE,
	                                                VK_CULL_MODE_FRONT_BIT);
	pipeline_state.m_depth_stencil_state.Enable(VK_TRUE, VK_TRUE);
	pipeline_state.m_multisample_state.Enable(VK_SAMPLE_COUNT_1_BIT);
	pipeline_state.m_viewport_state.Enable(
	    std::vector<VkViewport>{{0, 0, (float)m_resolution, (float)m_resolution, 0.0f, 1.0f}},
	    std::vector<VkRect2D>{{{0, 0}, {m_resolution, m_resolution}}});

	m_pipeline =
	    myvk::GraphicsPipeline::Create(pipeline_layout, GetVkRenderPass(), shader_stages, pipeline_state, GetSubpass());
}

void ShadowMapPass::CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const {
	command_buffer->CmdBindPipeline(m_pipeline);
	m_ani_instance.CmdDraw(command_buffer, m_draw_config);
}
} // namespace rg
