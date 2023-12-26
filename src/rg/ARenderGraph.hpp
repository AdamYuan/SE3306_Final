#pragma once

#include "../GPUAnimation.hpp"

#include "GBufferPass.hpp"
#include "LightPass.hpp"
#include "ShadowMapPass.hpp"
#include "VoxelMipmapPass.hpp"
#include "VoxelizePass.hpp"

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
		auto shadowmap_pass_0 = CreatePass<ShadowMapPass>({"shadow_pass", 0}, m_ani_instance,
		                                                  ADrawConfig{.opt_tumbler_lod = 0}, 480, nullptr);

		auto voxelize_pass =
		    CreatePass<VoxelizePass>({"voxelize_pass"}, m_ani_instance, 64, shadowmap_pass_0->GetShadowMapOutput());

		auto voxel_mipmap_pass =
		    CreatePass<VoxelMipmapPass>({"voxel_mipmap_pass"}, 64, 7, voxelize_pass->GetVoxelOutput());

		auto shadowmap_pass_1 =
		    CreatePass<ShadowMapPass>({"shadow_pass", 1}, m_ani_instance, ADrawConfig{.opt_marble_lod = 0}, 480,
		                              shadowmap_pass_0->GetShadowMapOutput());

		auto light_pass =
		    CreatePass<LightPass>({"light"}, gbuffer_pass->GetAlbedoOutput(), gbuffer_pass->GetNormalOutput(),
		                          gbuffer_pass->GetDepthOutput(), shadowmap_pass_1->GetShadowMapOutput(),
		                          voxelize_pass->GetVoxelOutput(), voxel_mipmap_pass->GetVoxelMipmapOutputs());

		auto blit_pass = CreatePass<myvk_rg::ImageBlitPass>({"blit_pass"}, light_pass->GetLightOutput(),
		                                                    swapchain_image, VK_FILTER_NEAREST);

		AddResult({"result"}, blit_pass->GetDstOutput());
	}

public:
	inline void Update(const Animation &animation) { m_ani_instance.Update(animation); }
};
} // namespace rg
