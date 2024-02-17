#pragma once

#include <glm/glm.hpp>
#include <myvk_rg/RenderGraph.hpp>

namespace rg {

class TAAPass final : public myvk_rg::GraphicsPassBase {
private:
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;
	glm::vec2 m_jitter = {};
	bool m_first = false;

public:
	inline auto GetTAAOutput() { return MakeImageOutput({"taa_in"}); }
	TAAPass(myvk_rg::Parent parent, const myvk_rg::Image &velocity, const myvk_rg::Image &light)
	    : GraphicsPassBase(parent) {
		const auto &device = GetRenderGraphPtr()->GetDevicePtr();
		auto sampler_edge = myvk::Sampler::Create(device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

		auto taa = CreateResource<myvk_rg::ManagedImage>({"taa"}, VK_FORMAT_A2R10G10B10_UNORM_PACK32);
		auto prev_taa = CreateResource<myvk_rg::LastFrameImage>({"prev_taa"});

		AddColorAttachmentInput<myvk_rg::Usage::kColorAttachmentW>(0, {"taa_in"}, taa->AsInput());

		AddInputAttachmentInput(0, {0}, {"velocity_in"}, velocity);
		AddDescriptorInput<myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>({1}, {"light_in"},
		                                                                                           light, sampler_edge);
		AddDescriptorInput<myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>(
		    {2}, {"prev_taa_in"}, prev_taa->AsInput(), sampler_edge);

		prev_taa->SetPointedAlias(GetTAAOutput());
	}

	inline ~TAAPass() final = default;
	inline void SetJitter(glm::vec2 jitter) { m_jitter = jitter; }
	inline void SetFirst(bool first) { m_first = first; }
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
};

} // namespace rg
