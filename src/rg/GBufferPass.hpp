#pragma once

#include "../GPUAnimation.hpp"
#include <myvk_rg/RenderGraph.hpp>

namespace rg {
class GBufferPass final : public myvk_rg::GraphicsPassBase {
	MYVK_RG_FRIENDS

private:
	GPUAInstance m_ani_instance;
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;

	void Initialize(const GPUAInstance &ani_instance) {
		m_ani_instance = ani_instance;

		auto albedo = CreateResource<myvk_rg::ManagedImage>({"albedo"}, VK_FORMAT_R16G16B16A16_SFLOAT);
		auto normal = CreateResource<myvk_rg::ManagedImage>({"normal"}, VK_FORMAT_R16G16_UNORM);
		auto velocity = CreateResource<myvk_rg::ManagedImage>({"velocity"}, VK_FORMAT_R16G16_SFLOAT);
		auto depth = CreateResource<myvk_rg::ManagedImage>({"depth"}, VK_FORMAT_D32_SFLOAT);
		depth->SetLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);
		depth->SetClearDepthStencilValue({.depth = 1.0f});

		AddColorAttachmentInput<0, myvk_rg::Usage::kColorAttachmentW>({"albedo_in"}, albedo);
		AddColorAttachmentInput<1, myvk_rg::Usage::kColorAttachmentW>({"normal_in"}, normal);
		AddColorAttachmentInput<2, myvk_rg::Usage::kColorAttachmentW>({"velocity_in"}, velocity);
		SetDepthAttachmentInput<myvk_rg::Usage::kDepthAttachmentRW>({"depth_in"}, depth);
	}

public:
	~GBufferPass() final;
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline auto GetAlbedoOutput() { return MakeImageOutput({"albedo_in"}); }
	inline auto GetNormalOutput() { return MakeImageOutput({"normal_in"}); }
	inline auto GetVelocityOutput() { return MakeImageOutput({"velocity_in"}); }
	inline auto GetDepthOutput() { return MakeImageOutput({"depth_in"}); }
};
} // namespace rg
