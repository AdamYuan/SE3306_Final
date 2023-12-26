#pragma once

#include <glm/glm.hpp>
#include <myvk_rg/RenderGraph.hpp>

namespace rg {

class TAAPass final : public myvk_rg::GraphicsPassBase {
	MYVK_RG_FRIENDS

private:
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;
	glm::vec2 m_jitter = {};
	bool m_first = false;

	void Initialize(myvk_rg::ImageInput velocity, myvk_rg::ImageInput light) {
		const auto &device = GetRenderGraphPtr()->GetDevicePtr();
		auto sampler_edge = myvk::Sampler::Create(device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

		auto taa = CreateResource<myvk_rg::ManagedImage>({"taa"}, VK_FORMAT_A2R10G10B10_UNORM_PACK32);
		auto prev_taa = CreateResource<myvk_rg::LastFrameImage>({"prev_taa"}, taa);

		AddColorAttachmentInput<0, myvk_rg::Usage::kColorAttachmentW>({"taa_in"}, taa);
		AddDescriptorInput<0, myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>(
		    {"velocity_in"}, velocity, sampler_edge);
		AddDescriptorInput<1, myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>(
		    {"light_in"}, light, sampler_edge);
		AddDescriptorInput<2, myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>(
		    {"prev_taa_in"}, prev_taa, sampler_edge);
	}

public:
	inline ~TAAPass() final = default;
	inline void SetJitter(glm::vec2 jitter) { m_jitter = jitter; }
	inline void SetFirst(bool first) { m_first = first; }
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline auto GetTAAOutput() { return MakeImageOutput({"taa_in"}); }
};

} // namespace rg
