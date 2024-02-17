#pragma once

#include "../GPUAnimation.hpp"
#include <myvk_rg/RenderGraph.hpp>

namespace rg {
class ShadowMapPass final : public myvk_rg::GraphicsPassBase {
private:
	GPUAInstance m_ani_instance;
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;
	ADrawConfig m_draw_config;
	uint32_t m_resolution;

public:
	ShadowMapPass(myvk_rg::Parent parent, const GPUAInstance &ani_instance, const ADrawConfig &draw_config,
	              uint32_t resolution, const myvk_rg::Image &depth_input = {})
	    : GraphicsPassBase(parent) {
		m_ani_instance = ani_instance;
		m_draw_config = draw_config;
		m_resolution = resolution;

		if (depth_input) {
			AddDepthAttachmentInput<myvk_rg::Usage::kDepthAttachmentRW>({"depth_in"}, depth_input);
		} else {
			auto depth = CreateResource<myvk_rg::ManagedImage>({"depth"}, VK_FORMAT_D32_SFLOAT);
			depth->SetSize2D({resolution, resolution});
			depth->SetLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);
			depth->SetClearDepthStencilValue({.depth = 1.0f});

			AddDepthAttachmentInput<myvk_rg::Usage::kDepthAttachmentRW>({"depth_in"}, depth->AsInput());
		}
	}

	inline ~ShadowMapPass() final = default;
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline auto GetShadowMapOutput() { return MakeImageOutput({"depth_in"}); }
};
} // namespace rg
