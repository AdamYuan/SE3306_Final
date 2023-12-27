#include "TAAPass.hpp"

namespace rg {

void TAAPass::CreatePipeline() {
	auto pipeline_layout = myvk::PipelineLayout::Create(
	    GetRenderGraphPtr()->GetDevicePtr(), {GetVkDescriptorSetLayout()},
	    {VkPushConstantRange{
	        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .offset = 0, .size = sizeof(int32_t) + sizeof(glm::vec2)}});

	const auto &device = GetRenderGraphPtr()->GetDevicePtr();

	constexpr uint32_t kVertSpv[] = {
#include <shader/quad.vert.u32>
	};
	constexpr uint32_t kFragSpv[] = {
#include <shader/taa.frag.u32>
	};

	std::shared_ptr<myvk::ShaderModule> vert_shader_module, frag_shader_module;
	vert_shader_module = myvk::ShaderModule::Create(device, kVertSpv, sizeof(kVertSpv));
	frag_shader_module = myvk::ShaderModule::Create(device, kFragSpv, sizeof(kFragSpv));

	std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {
	    vert_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
	    frag_shader_module->GetPipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)};

	myvk::GraphicsPipelineState pipeline_state = {};
	auto extent = GetRenderGraphPtr()->GetCanvasSize();
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
void TAAPass::CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const {
	command_buffer->CmdBindPipeline(m_pipeline);
	static_assert(sizeof(float) == sizeof(int32_t));
	float pc_data[3];
	*(glm::vec2 *)pc_data = glm::vec2(m_jitter.x, -m_jitter.y) * 0.5f;
	*(int32_t *)(pc_data + 2) = m_first;
	command_buffer->CmdBindDescriptorSets({GetVkDescriptorSet()}, m_pipeline);
	command_buffer->CmdPushConstants(m_pipeline->GetPipelineLayoutPtr(), VK_SHADER_STAGE_FRAGMENT_BIT, 0,
	                                 sizeof(pc_data), pc_data);
	command_buffer->CmdDraw(3, 1, 0, 0);
}

} // namespace rg