#pragma once

#include <array>
#include <myvk_rg/RenderGraph.hpp>

namespace rg {

class LightPass final : public myvk_rg::GraphicsPassBase {
private:
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;
	uint32_t m_tick = 0;

public:
	LightPass(myvk_rg::Parent parent, const myvk_rg::Image &albedo, const myvk_rg::Image &normal,
	          const myvk_rg::Image &depth, const myvk_rg::Image &shadow_map, const myvk_rg::Image &voxel,
	          std::array<myvk_rg::Image, 6> voxel_mipmaps)
	    : myvk_rg::GraphicsPassBase(parent) {
		const auto &device = GetRenderGraphPtr()->GetDevicePtr();
		auto sampler_edge = myvk::Sampler::Create(device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		auto sampler_border = myvk::Sampler::CreateClampToBorder(
		    device, VK_FILTER_LINEAR, VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK, VK_SAMPLER_MIPMAP_MODE_LINEAR);

		VkSamplerCreateInfo shadow_sampler_info = {};
		shadow_sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		shadow_sampler_info.magFilter = VK_FILTER_LINEAR;
		shadow_sampler_info.minFilter = VK_FILTER_LINEAR;
		shadow_sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		shadow_sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		shadow_sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		shadow_sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		shadow_sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		shadow_sampler_info.mipLodBias = 0.0f;
		shadow_sampler_info.compareOp = VK_COMPARE_OP_LESS;
		shadow_sampler_info.compareEnable = VK_TRUE;
		shadow_sampler_info.minLod = 0.0f;
		shadow_sampler_info.maxLod = VK_LOD_CLAMP_NONE;
		auto sampler_shadow = myvk::Sampler::Create(device, shadow_sampler_info);

		auto light = CreateResource<myvk_rg::ManagedImage>({"light"}, VK_FORMAT_A2R10G10B10_UNORM_PACK32);

		AddColorAttachmentInput<myvk_rg::Usage::kColorAttachmentW>(0, {"light_in"}, light->Alias());
		AddInputAttachmentInput(0, {0}, {"albedo_in"}, albedo);
		AddInputAttachmentInput(1, {1}, {"normal_in"}, normal);
		AddInputAttachmentInput(2, {2}, {"depth_in"}, depth);
		AddDescriptorInput<myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>(
		    {3}, {"sm_in"}, shadow_map, sampler_shadow);
		AddDescriptorInput<myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>(
		    {4}, {"voxel_in"}, voxel, sampler_border);
		for (uint32_t i = 0; i < 6; ++i)
			AddDescriptorInput<myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>(
			    {5, i}, {"voxel_mip_in", i}, voxel_mipmaps[i], sampler_border);
	}
	inline ~LightPass() final = default;
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline auto GetLightOutput() { return MakeImageOutput({"light_in"}); }
	inline void SetTick(uint32_t tick) { m_tick = tick; }
};

} // namespace rg
