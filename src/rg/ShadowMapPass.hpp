#pragma once

#include "../GPUAnimation.hpp"
#include <myvk_rg/RenderGraph.hpp>

namespace rg {
class ShadowMapPass final : public myvk_rg::GraphicsPassBase {
	MYVK_RG_FRIENDS

private:
	GPUAInstance m_ani_instance;
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;
	ADrawConfig m_draw_config;
	uint32_t m_resolution;

	void Initialize(const GPUAInstance &ani_instance, const ADrawConfig &draw_config, uint32_t resolution,
	                myvk_rg::ImageInput depth_input = nullptr) {
		m_ani_instance = ani_instance;
		m_draw_config = draw_config;
		m_resolution = resolution;

		if (depth_input) {
			SetDepthAttachmentInput<myvk_rg::Usage::kDepthAttachmentRW>({"depth_in"}, depth_input);
		} else {
			auto depth = CreateResource<myvk_rg::ManagedImage>({"depth"}, VK_FORMAT_D32_SFLOAT);
			depth->SetSize2D({resolution, resolution});
			depth->SetLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);
			depth->SetClearDepthStencilValue({.depth = 1.0f});

			SetDepthAttachmentInput<myvk_rg::Usage::kDepthAttachmentRW>({"depth_in"}, depth);
		}
	}

public:
	~ShadowMapPass() final;
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline auto GetShadowMapOutput() { return MakeImageOutput({"depth_in"}); }
};
} // namespace rg
