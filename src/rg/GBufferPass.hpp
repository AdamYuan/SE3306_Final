#pragma once

#include "../GPUAnimation.hpp"
#include <glm/glm.hpp>
#include <myvk_rg/RenderGraph.hpp>

namespace rg {
class GBufferPass final : public myvk_rg::GraphicsPassBase {
private:
	GPUAInstance m_ani_instance;
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;
	glm::vec2 m_jitter = {};

public:
	GBufferPass(myvk_rg::Parent parent, const GPUAInstance &ani_instance) : myvk_rg::GraphicsPassBase(parent) {
		m_ani_instance = ani_instance;

		auto albedo = CreateResource<myvk_rg::ManagedImage>({"albedo"}, VK_FORMAT_R16G16B16A16_SFLOAT);
		auto normal = CreateResource<myvk_rg::ManagedImage>({"normal"}, VK_FORMAT_R8G8_SNORM);
		auto velocity = CreateResource<myvk_rg::ManagedImage>({"velocity"}, VK_FORMAT_R16G16_SFLOAT);
		auto depth = CreateResource<myvk_rg::ManagedImage>({"depth"}, VK_FORMAT_D32_SFLOAT);
		depth->SetLoadOp(VK_ATTACHMENT_LOAD_OP_CLEAR);
		depth->SetClearDepthStencilValue({.depth = 1.0f});

		AddColorAttachmentInput<myvk_rg::Usage::kColorAttachmentW>(0, {"albedo_in"}, albedo->Alias());
		AddColorAttachmentInput<myvk_rg::Usage::kColorAttachmentW>(1, {"normal_in"}, normal->Alias());
		AddColorAttachmentInput<myvk_rg::Usage::kColorAttachmentW>(2, {"velocity_in"}, velocity->Alias());
		AddDepthAttachmentInput<myvk_rg::Usage::kDepthAttachmentRW>({"depth_in"}, depth->Alias());
	}

	inline ~GBufferPass() final = default;
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline void SetJitter(glm::vec2 jitter) { m_jitter = jitter; }
	inline auto GetAlbedoOutput() { return MakeImageOutput({"albedo_in"}); }
	inline auto GetNormalOutput() { return MakeImageOutput({"normal_in"}); }
	inline auto GetVelocityOutput() { return MakeImageOutput({"velocity_in"}); }
	inline auto GetDepthOutput() { return MakeImageOutput({"depth_in"}); }
};
} // namespace rg
