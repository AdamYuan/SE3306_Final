#include "ARenderGraph.hpp"

#include "BloomPass.hpp"
#include "GBufferPass.hpp"
#include "LightPass.hpp"
#include "MBTilePass.hpp"
#include "MotionBlurPass.hpp"
#include "ScreenPass.hpp"
#include "ShadowMapPass.hpp"
#include "TAAPass.hpp"
#include "VoxelMipmapPass.hpp"
#include "VoxelizePass.hpp"

#include <GLFW/glfw3.h>

namespace rg {

void ARenderGraph::create_passes() {
	ClearPasses();
	ClearResults();

	auto gbuffer_pass = CreatePass<GBufferPass>({"gbuffer_pass"}, m_ani_instance);
	auto shadowmap_pass_0 =
	    CreatePass<ShadowMapPass>({"shadow_pass", 0}, m_ani_instance, ADrawConfig{.opt_tumbler_lod = 0}, 480, nullptr);

	auto voxelize_pass =
	    CreatePass<VoxelizePass>({"voxelize_pass"}, m_ani_instance, 64, shadowmap_pass_0->GetShadowMapOutput());

	auto voxel_mipmap_pass = CreatePass<VoxelMipmapPass>({"voxel_mipmap_pass"}, 64, 7, voxelize_pass->GetVoxelOutput());

	auto shadowmap_pass_1 =
	    CreatePass<ShadowMapPass>({"shadow_pass", 1}, m_ani_instance, ADrawConfig{.opt_marble_lod = 0}, 480,
	                              shadowmap_pass_0->GetShadowMapOutput());

	auto light_pass =
	    CreatePass<LightPass>({"light_pass"}, gbuffer_pass->GetAlbedoOutput(), gbuffer_pass->GetNormalOutput(),
	                          gbuffer_pass->GetDepthOutput(), shadowmap_pass_1->GetShadowMapOutput(),
	                          voxelize_pass->GetVoxelOutput(), voxel_mipmap_pass->GetVoxelMipmapOutputs());

	auto taa_pass = CreatePass<TAAPass>({"taa_pass"}, gbuffer_pass->GetVelocityOutput(), light_pass->GetLightOutput());

	auto mb_tile_pass = CreatePass<MBTilePass>({"mb_tile_pass"}, gbuffer_pass->GetVelocityOutput(), 16);

	auto mb_pass =
	    CreatePass<MotionBlurPass>({"mb_pass"}, taa_pass->GetTAAOutput(), mb_tile_pass->GetVelocityTileOutput(),
	                               gbuffer_pass->GetVelocityOutput(), gbuffer_pass->GetDepthOutput());

	auto bloom_pass = CreatePass<BloomPass>({"bloom_pass"}, gbuffer_pass->GetAlbedoOutput(), 6, 0.005f);

	auto swapchain_image = GetResource<myvk_rg::SwapchainImage>({"swapchain_image"});

	auto screen_pass =
	    CreatePass<ScreenPass>({"screen_pass"}, m_enable_mb ? mb_pass->GetMotionBlurOutput() : taa_pass->GetTAAOutput(),
	                           bloom_pass->GetBloomOutput(), swapchain_image);
	// auto blit_pass = CreatePass<myvk_rg::ImageBlitPass>({"blit_pass"}, bloom_pass->GetBloomOutput(), swapchain_image,
	//                                                     VK_FILTER_NEAREST);

	AddResult({"result"}, screen_pass->GetOutput());
}

void ARenderGraph::Initialize(const myvk::Ptr<myvk::FrameManager> &frame_manager, const GPUAInstance &gpu_ani_instance,
                              uint64_t tick_mask) {
	m_ani_instance = gpu_ani_instance;
	m_tick_mask = tick_mask;
	m_time = glfwGetTime();

	auto swapchain_image = CreateResource<myvk_rg::SwapchainImage>({"swapchain_image"}, frame_manager);
	create_passes();
}

void ARenderGraph::Update(const Animation &animation) {
	m_ani_instance.Update(animation);

	constexpr const glm::vec2 kJitters[16] = {
	    {0.500000, 0.333333}, {0.250000, 0.666667}, {0.750000, 0.111111}, {0.125000, 0.444444},
	    {0.625000, 0.777778}, {0.375000, 0.222222}, {0.875000, 0.555556}, {0.062500, 0.888889},
	    {0.562500, 0.037037}, {0.312500, 0.370370}, {0.812500, 0.703704}, {0.187500, 0.148148},
	    {0.687500, 0.481481}, {0.437500, 0.814815}, {0.937500, 0.259259}, {0.031250, 0.592593},
	};
	glm::vec2 jitter = (kJitters[(m_tick ^ m_tick_mask) & 0xfu] * 2.f - 1.f) /
	                   glm::vec2{GetCanvasSize().width, GetCanvasSize().height};
	GetPass<GBufferPass>({"gbuffer_pass"})->SetJitter(jitter);
	GetPass<TAAPass>({"taa_pass"})->SetJitter(jitter);
	GetPass<TAAPass>({"taa_pass"})->SetFirst(m_tick == 0);
	double new_time = glfwGetTime(), delta = new_time - m_time;
	GetPass<MotionBlurPass>({"mb_pass"})->SetJitter(jitter);
	GetPass<MotionBlurPass>({"mb_pass"})->SetSearchScale(glm::max(float(0.015 / delta), 1.0f));
	GetPass<ScreenPass>({"screen_pass"})->SetJitter(jitter);
	GetPass<LightPass>({"light_pass"})->SetTick(m_tick ^ m_tick_mask);

	++m_tick;
	m_time = new_time;
}

} // namespace rg
