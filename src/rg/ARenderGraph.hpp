#pragma once

#include "../GPUAnimation.hpp"

#include <myvk_rg/RenderGraph.hpp>
#include <myvk_rg/pass/ImageBlitPass.hpp>
#include <myvk_rg/resource/SwapchainImage.hpp>

#include <chrono>

namespace rg {
class ARenderGraph final : public myvk_rg::RenderGraphBase {
private:
	GPUAInstance m_ani_instance;
	uint64_t m_tick = 0, m_tick_mask = 0;
	double m_time;
	bool m_enable_mb;

	void create_passes();

public:
	ARenderGraph(const myvk::Ptr<myvk::FrameManager> &frame_manager, const GPUAInstance &gpu_ani_instance,
	             uint64_t tick_mask);

	void Update(const Animation &animation);
	void SetMotionBlur(bool enable) {
		if (m_enable_mb != enable) {
			m_enable_mb = enable;
			create_passes();
		}
	}
};
} // namespace rg
