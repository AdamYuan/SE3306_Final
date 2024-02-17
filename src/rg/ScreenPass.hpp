#pragma once

#include <glm/glm.hpp>
#include <myvk_rg/RenderGraph.hpp>

namespace rg {

class ScreenPass final : public myvk_rg::GraphicsPassBase {
private:
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;
	glm::vec2 m_jitter{};

public:
	inline ScreenPass(myvk_rg::Parent parent, const myvk_rg::Image &color, const myvk_rg::Image &bloom,
	                  const myvk_rg::Image &output)
	    : myvk_rg::GraphicsPassBase(parent) {
		const auto &device = GetRenderGraphPtr()->GetDevicePtr();
		auto sampler = myvk::Sampler::Create(device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		AddInputAttachmentInput(0, {0}, {"color_in"}, color);
		AddDescriptorInput<myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>({1}, {"bloom_in"},
		                                                                                           bloom, sampler);
		AddColorAttachmentInput<myvk_rg::Usage::kColorAttachmentW>(0, {"output_in"}, output);
	}

	inline ~ScreenPass() final = default;
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline auto GetOutput() { return MakeImageOutput({"output_in"}); }
	inline void SetJitter(glm::vec2 jitter) { m_jitter = jitter; }
};

} // namespace rg
