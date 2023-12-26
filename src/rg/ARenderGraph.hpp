#pragma once

#include "../GPUAnimation.hpp"

#include <myvk_rg/RenderGraph.hpp>
#include <myvk_rg/pass/ImageBlitPass.hpp>
#include <myvk_rg/resource/SwapchainImage.hpp>

namespace rg {
class ARenderGraph final : public myvk_rg::RenderGraph<ARenderGraph> {
	MYVK_RG_FRIENDS
private:
	GPUAInstance m_ani_instance;
	uint64_t m_tick = 0, m_tick_mask = 0;

	void Initialize(const myvk::Ptr<myvk::FrameManager> &frame_manager, const GPUAInstance &gpu_ani_instance,
	                uint64_t tick_mask);

public:
	void Update(const Animation &animation);
};
} // namespace rg
