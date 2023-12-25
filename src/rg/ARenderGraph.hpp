#pragma once

#include "../GPUAnimation.hpp"

#include "GBufferPass.hpp"
#include "ShadowMapPass.hpp"

#include <myvk_rg/RenderGraph.hpp>
#include <myvk_rg/pass/ImageBlitPass.hpp>
#include <myvk_rg/resource/SwapchainImage.hpp>

namespace rg {
class ARenderGraph final : public myvk_rg::RenderGraph<ARenderGraph> {
	MYVK_RG_FRIENDS
private:
	GPUAInstance m_ani_instance;
	void Initialize(const myvk::Ptr<myvk::FrameManager> &frame_manager, const GPUAInstance &gpu_ani_instance) {
		m_ani_instance = gpu_ani_instance;

		auto swapchain_image = CreateResource<myvk_rg::SwapchainImage>({"swapchain_image"}, frame_manager);

		auto gbuffer_pass = CreatePass<GBufferPass>({"gbuffer_pass"}, m_ani_instance);
		auto shadowmap_pass_0 =
		    CreatePass<ShadowMapPass>({"shadow_pass", 0}, m_ani_instance, ADrawConfig{.opt_tumbler_lod = 0}, nullptr);

		auto shadowmap_pass_1 =
		    CreatePass<ShadowMapPass>({"shadow_pass", 1}, m_ani_instance, ADrawConfig{.opt_marble_lod = 0},
		                              shadowmap_pass_0->GetShadowMapOutput());

		auto blit_pass = CreatePass<myvk_rg::ImageBlitPass>({"blit"}, gbuffer_pass->GetAlbedoOutput(), swapchain_image,
		                                                    VK_FILTER_NEAREST);
		AddResult({"result"}, blit_pass->GetDstOutput());
	}

public:
	inline void Update(const Animation &animation) { m_ani_instance.Update(animation); }
};
} // namespace rg
