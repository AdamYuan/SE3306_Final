#pragma once

#include <glm/glm.hpp>
#include <myvk_rg/RenderGraph.hpp>

namespace rg {

class MBSpeedDepthPass final : public myvk_rg::GraphicsPassBase {
private:
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;

public:
	MBSpeedDepthPass(myvk_rg::Parent parent, const myvk_rg::Image &velocity, const myvk_rg::Image &depth)
	    : myvk_rg::GraphicsPassBase(parent) {
		auto speed_depth = CreateResource<myvk_rg::ManagedImage>({"sd"}, VK_FORMAT_R16G16_SFLOAT);
		AddInputAttachmentInput(0, {0}, {"velocity"}, velocity);
		AddInputAttachmentInput(1, {1}, {"depth"}, depth);
		AddColorAttachmentInput<myvk_rg::Usage::kColorAttachmentW>(0, {"sd_in"}, speed_depth->AsInput());
	}

	inline ~MBSpeedDepthPass() final = default;
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline auto GetSpeedDepthOutput() { return MakeImageOutput({"sd_in"}); }
};

class MBDrawPass final : public myvk_rg::GraphicsPassBase {
private:
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;
	glm::vec2 m_jitter{};
	float m_search_scale{1.0f};

public:
	MBDrawPass(myvk_rg::Parent parent, const myvk_rg::Image &taa, const myvk_rg::Image &tile,
	           const myvk_rg::Image &speed_depth)
	    : myvk_rg::GraphicsPassBase(parent) {
		const auto &device = GetRenderGraphPtr()->GetDevicePtr();
		auto sampler_edge = myvk::Sampler::Create(device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		AddDescriptorInput<myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>({0}, {"taa_in"}, taa,
		                                                                                           sampler_edge);
		AddDescriptorInput<myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>(
		    {1}, {"sd_in"}, speed_depth, sampler_edge);
		AddDescriptorInput<myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>({2}, {"tile_in"},
		                                                                                           tile, sampler_edge);

		auto mb = CreateResource<myvk_rg::ManagedImage>({"mb"}, VK_FORMAT_A2R10G10B10_UNORM_PACK32);
		AddColorAttachmentInput<myvk_rg::Usage::kColorAttachmentW>(0, {"mb_in"}, mb->AsInput());
	}
	inline ~MBDrawPass() final = default;
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline auto GetMotionBlurOutput() { return MakeImageOutput({"mb_in"}); }

	inline void SetJitter(glm::vec2 jitter) { m_jitter = jitter; }
	inline void SetSearchScale(float search_scale) { m_search_scale = search_scale; }
};

class MotionBlurPass final : public myvk_rg::PassGroupBase {
public:
	inline MotionBlurPass(myvk_rg::Parent parent, const myvk_rg::Image &taa, const myvk_rg::Image &tile,
	                      const myvk_rg::Image &velocity, const myvk_rg::Image &depth)
	    : myvk_rg::PassGroupBase(parent) {
		auto speed_depth_pass = CreatePass<MBSpeedDepthPass>({"sd_pass"}, velocity, depth);
		CreatePass<MBDrawPass>({"draw_pass"}, taa, tile, speed_depth_pass->GetSpeedDepthOutput());
	}
	inline ~MotionBlurPass() final = default;
	inline auto GetMotionBlurOutput() { return GetPass<MBDrawPass>({"draw_pass"})->GetMotionBlurOutput(); }

	inline void SetJitter(glm::vec2 jitter) { GetPass<MBDrawPass>({"draw_pass"})->SetJitter(jitter); }
	inline void SetSearchScale(float search_scale) { GetPass<MBDrawPass>({"draw_pass"})->SetSearchScale(search_scale); }
};

} // namespace rg
