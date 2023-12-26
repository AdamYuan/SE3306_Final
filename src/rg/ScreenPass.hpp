#pragma once

#include <glm/glm.hpp>
#include <myvk_rg/RenderGraph.hpp>

namespace rg {

class ScreenPass final : public myvk_rg::GraphicsPassBase {
	MYVK_RG_FRIENDS

private:
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;
	glm::vec2 m_jitter;

	inline void Initialize(myvk_rg::ImageInput color, myvk_rg::ImageInput bloom, myvk_rg::ImageInput output) {
		const auto &device = GetRenderGraphPtr()->GetDevicePtr();
		auto sampler = myvk::Sampler::Create(device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		AddInputAttachmentInput<0, 0>({"color_in"}, color);
		AddDescriptorInput<1, myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>({"bloom_in"},
		                                                                                              bloom, sampler);
		AddColorAttachmentInput<0, myvk_rg::Usage::kColorAttachmentW>({"output_in"}, output);
	}

public:
	inline ~ScreenPass() final = default;
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline auto GetOutput() { return MakeImageOutput({"output_in"}); }
	inline void SetJitter(glm::vec2 jitter) { m_jitter = jitter; }
};

} // namespace rg
