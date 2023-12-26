#pragma once

#include <glm/glm.hpp>
#include <myvk_rg/RenderGraph.hpp>

namespace rg {

class MBSpeedDepthPass final : public myvk_rg::GraphicsPassBase {
	MYVK_RG_FRIENDS

private:
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;

	void Initialize(myvk_rg::ImageInput velocity, myvk_rg::ImageInput depth) {
		auto speed_depth = CreateResource<myvk_rg::ManagedImage>({"sd"}, VK_FORMAT_R16G16_SFLOAT);
		AddInputAttachmentInput<0, 0>({"velocity"}, velocity);
		AddInputAttachmentInput<1, 1>({"depth"}, depth);
		AddColorAttachmentInput<0, myvk_rg::Usage::kColorAttachmentW>({"sd_in"}, speed_depth);
	}

public:
	inline ~MBSpeedDepthPass() final = default;
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline auto GetSpeedDepthOutput() { return MakeImageOutput({"sd_in"}); }
};

class MBDrawPass final : public myvk_rg::GraphicsPassBase {
	MYVK_RG_FRIENDS

private:
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;
	glm::vec2 m_jitter{};
	float m_search_scale{1.0f};

	void Initialize(myvk_rg::ImageInput taa, myvk_rg::ImageInput tile, myvk_rg::ImageInput speed_depth) {
		const auto &device = GetRenderGraphPtr()->GetDevicePtr();
		auto sampler_edge = myvk::Sampler::Create(device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		AddDescriptorInput<0, myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>({"taa_in"}, taa,
		                                                                                              sampler_edge);
		AddDescriptorInput<1, myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>(
		    {"sd_in"}, speed_depth, sampler_edge);
		AddDescriptorInput<2, myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>({"tile_in"}, tile,
		                                                                                              sampler_edge);

		auto mb = CreateResource<myvk_rg::ManagedImage>({"mb"}, VK_FORMAT_A2R10G10B10_UNORM_PACK32);
		AddColorAttachmentInput<0, myvk_rg::Usage::kColorAttachmentW>({"mb_in"}, mb);
	}

public:
	inline ~MBDrawPass() final = default;
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline auto GetMotionBlurOutput() { return MakeImageOutput({"mb_in"}); }

	inline void SetJitter(glm::vec2 jitter) { m_jitter = jitter; }
	inline void SetSearchScale(float search_scale) { m_search_scale = search_scale; }
};

class MotionBlurPass final : public myvk_rg::PassGroupBase {
	MYVK_RG_FRIENDS
private:
	inline void Initialize(myvk_rg::ImageInput taa, myvk_rg::ImageInput tile, //
	                       myvk_rg::ImageInput velocity, myvk_rg::ImageInput depth) {
		auto speed_depth_pass = CreatePass<MBSpeedDepthPass>({"sd_pass"}, velocity, depth);
		CreatePass<MBDrawPass>({"draw_pass"}, taa, tile, speed_depth_pass->GetSpeedDepthOutput());
	}

public:
	inline ~MotionBlurPass() final = default;
	inline auto GetMotionBlurOutput() {
		return CreateImageAliasOutput({"out"}, GetPass<MBDrawPass>({"draw_pass"})->GetMotionBlurOutput());
	}

	inline void SetJitter(glm::vec2 jitter) { GetPass<MBDrawPass>({"draw_pass"})->SetJitter(jitter); }
	inline void SetSearchScale(float search_scale) { GetPass<MBDrawPass>({"draw_pass"})->SetSearchScale(search_scale); }
};

} // namespace rg
