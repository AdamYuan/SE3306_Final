#pragma once

#include <myvk_rg/RenderGraph.hpp>

namespace rg {

class BloomDownPass1 final : public myvk_rg::GraphicsPassBase {
private:
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;

public:
	inline BloomDownPass1(myvk_rg::Parent parent, const myvk_rg::Image &albedo) : myvk_rg::GraphicsPassBase(parent) {
		const auto &device = GetRenderGraphPtr()->GetDevicePtr();

		auto sampler = myvk::Sampler::Create(device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);

		AddDescriptorInput<myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>({0}, {"albedo_in"},
		                                                                                           albedo, sampler);

		auto down = CreateResource<myvk_rg::ManagedImage>({"down"}, VK_FORMAT_R16G16B16A16_SFLOAT);
		down->SetSizeFunc([](VkExtent2D c) {
			return myvk_rg::SubImageSize{VkExtent2D{std::max(c.width >> 1u, 1u), std::max(c.height >> 1u, 1u)}};
		});

		AddColorAttachmentInput<myvk_rg::Usage::kColorAttachmentW>(0, {"down_in"}, down->Alias());
	}
	inline ~BloomDownPass1() final = default;
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline auto GetDownOutput() { return MakeImageOutput({"down_in"}); }
};

class BloomDownPass final : public myvk_rg::GraphicsPassBase {
private:
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;
	uint32_t m_level{};

public:
	inline BloomDownPass(myvk_rg::Parent parent, uint32_t level, const myvk_rg::Image &upper)
	    : myvk_rg::GraphicsPassBase(parent) {
		m_level = level;
		const auto &device = GetRenderGraphPtr()->GetDevicePtr();
		auto sampler = myvk::Sampler::Create(device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		AddDescriptorInput<myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>({0}, {"upper_in"},
		                                                                                           upper, sampler);
		auto down = CreateResource<myvk_rg::ManagedImage>({"down"}, VK_FORMAT_R16G16B16A16_SFLOAT);
		down->SetSizeFunc([level](VkExtent2D c) {
			return myvk_rg::SubImageSize{VkExtent2D{std::max(c.width >> level, 1u), std::max(c.height >> level, 1u)}};
		});

		AddColorAttachmentInput<myvk_rg::Usage::kColorAttachmentW>(0, {"down_in"}, down->Alias());
	}
	inline ~BloomDownPass() final = default;
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline auto GetDownOutput() { return MakeImageOutput({"down_in"}); }
};

class BloomUpPass final : public myvk_rg::GraphicsPassBase {
private:
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;
	float m_filter_radius{};
	uint32_t m_level{};

public:
	inline BloomUpPass(myvk_rg::Parent parent, uint32_t level, const myvk_rg::Image &current,
	                   const myvk_rg::Image &lower, float filter_radius)
	    : myvk_rg::GraphicsPassBase(parent) {
		m_filter_radius = filter_radius;
		m_level = level;
		const auto &device = GetRenderGraphPtr()->GetDevicePtr();
		auto sampler = myvk::Sampler::Create(device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		AddDescriptorInput<myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>({0}, {"lower_in"},
		                                                                                           lower, sampler);
		AddColorAttachmentInput<myvk_rg::Usage::kColorAttachmentRW>(0, {"current_in"}, current);
	}
	inline ~BloomUpPass() final = default;
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline auto GetUpOutput() { return MakeImageOutput({"current_in"}); }
};

class BloomUpPass0 final : public myvk_rg::GraphicsPassBase {
private:
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;
	float m_filter_radius{};

public:
	inline BloomUpPass0(myvk_rg::Parent parent, const myvk_rg::Image &albedo, const myvk_rg::Image &lower,
	                    float filter_radius)
	    : myvk_rg::GraphicsPassBase(parent) {
		m_filter_radius = filter_radius;
		const auto &device = GetRenderGraphPtr()->GetDevicePtr();
		auto sampler = myvk::Sampler::Create(device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		AddDescriptorInput<myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>({0}, {"lower_in"},
		                                                                                           lower, sampler);
		AddInputAttachmentInput(0, {1}, {"albedo_in"}, albedo);
		auto current = CreateResource<myvk_rg::ManagedImage>({"current"}, VK_FORMAT_R16G16B16A16_SFLOAT);
		AddColorAttachmentInput<myvk_rg::Usage::kColorAttachmentW>(0, {"current_in"}, current->Alias());
	}

	inline ~BloomUpPass0() final = default;
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline auto GetUpOutput() { return MakeImageOutput({"current_in"}); }
};

class BloomPass final : public myvk_rg::PassGroupBase {
public:
	inline BloomPass(myvk_rg::Parent parent, const myvk_rg::Image &albedo, uint32_t levels, float filter_radius)
	    : myvk_rg::PassGroupBase(parent) {
		assert(levels > 1);
		std::vector<myvk_rg::Image> outputs(levels);
		outputs[1] = CreatePass<BloomDownPass1>({"down_pass", 1}, albedo)->GetDownOutput();
		for (uint32_t l = 2; l < levels; ++l)
			outputs[l] = CreatePass<BloomDownPass>({"down_pass", l}, l, outputs[l - 1])->GetDownOutput();
		for (uint32_t l = levels - 2; l != 0; --l)
			outputs[l] =
			    CreatePass<BloomUpPass>({"up_pass", l}, l, outputs[l], outputs[l + 1], filter_radius)->GetUpOutput();
		CreatePass<BloomUpPass0>({"up_pass", 0}, albedo, outputs[1], filter_radius);
	}
	inline ~BloomPass() final = default;
	inline auto GetBloomOutput() { return GetPass<BloomUpPass0>({"up_pass", 0})->GetUpOutput(); }
};

} // namespace rg
