#pragma once

#include <myvk_rg/RenderGraph.hpp>

namespace rg {

class MBTileMaxPass final : public myvk_rg::ComputePassBase {
	MYVK_RG_FRIENDS

private:
	myvk::Ptr<myvk::ComputePipeline> m_pipeline;
	uint32_t m_tile_size{}, m_subgroup_size{}, m_shared_size{};

	inline static constexpr auto div_ceil(auto x, auto y) { return x / y + (x % y == 0 ? 0 : 1); }

	inline void Initialize(myvk_rg::ImageInput velocity, uint32_t tile_size) {
		const auto &device = GetRenderGraphPtr()->GetDevicePtr();

		m_tile_size = tile_size;
		/* auto required_subgroup_operations =
		    VK_SUBGROUP_FEATURE_BASIC_BIT | VK_SUBGROUP_FEATURE_BALLOT_BIT | VK_SUBGROUP_FEATURE_ARITHMETIC_BIT;
		const auto &device_props = device->GetPhysicalDevicePtr()->GetProperties();
		const auto &device_features = device->GetPhysicalDevicePtr()->GetFeatures();
		if ((device_props.vk11.subgroupSupportedOperations & required_subgroup_operations) ==
		required_subgroup_operations) m_subgroup_size =
		device->GetPhysicalDevicePtr()->GetProperties().vk11.subgroupSize; else */
		m_subgroup_size = 1;
		m_shared_size = div_ceil(tile_size * tile_size, m_subgroup_size);

		auto sampler = myvk::Sampler::Create(device, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		AddDescriptorInput<0, myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT>({"velocity_in"},
		                                                                                             velocity, sampler);

		auto velocity_tile = CreateResource<myvk_rg::ManagedImage>({"vel_tile"}, VK_FORMAT_R16G16_SFLOAT);
		velocity_tile->SetSizeFunc([tile_size](const VkExtent2D &canvas_size) {
			return myvk_rg::SubImageSize{
			    VkExtent2D{div_ceil(canvas_size.width, tile_size), div_ceil(canvas_size.height, tile_size)}};
		});

		AddDescriptorInput<1, myvk_rg::Usage::kStorageImageW, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT>({"vel_tile_in"},
		                                                                                              velocity_tile);
	}

public:
	inline ~MBTileMaxPass() final = default;
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline auto GetVelocityTileOutput() { return MakeImageOutput({"vel_tile_in"}); }
};

class MBTileNeiPass final : public myvk_rg::GraphicsPassBase {
	MYVK_RG_FRIENDS

private:
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;
	uint32_t m_tile_size{};

	inline static constexpr auto div_ceil(auto x, auto y) { return x / y + (x % y == 0 ? 0 : 1); }

	inline void Initialize(myvk_rg::ImageInput max_velocity_tile, uint32_t tile_size) {
		const auto &device = GetRenderGraphPtr()->GetDevicePtr();

		m_tile_size = tile_size;

		auto sampler = myvk::Sampler::Create(device, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		AddDescriptorInput<0, myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>(
		    {"max_tile_in"}, max_velocity_tile, sampler);

		auto nei_velocity_tile = CreateResource<myvk_rg::ManagedImage>({"nei_tile"}, VK_FORMAT_R16G16_SFLOAT);
		nei_velocity_tile->SetSizeFunc([tile_size](const VkExtent2D &canvas_size) {
			return myvk_rg::SubImageSize{
			    VkExtent2D{div_ceil(canvas_size.width, tile_size), div_ceil(canvas_size.height, tile_size)}};
		});

		AddColorAttachmentInput<0, myvk_rg::Usage::kColorAttachmentW>({"nei_tile_in"}, nei_velocity_tile);
	}

public:
	inline ~MBTileNeiPass() final = default;
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline auto GetVelocityTileOutput() { return MakeImageOutput({"nei_tile_in"}); }
};

class MBTilePass final : public myvk_rg::PassGroupBase {
	MYVK_RG_FRIENDS
private:
	inline void Initialize(myvk_rg::ImageInput velocity, uint32_t tile_size) {
		auto max_pass = CreatePass<MBTileMaxPass>({"max_pass"}, velocity, tile_size);
		CreatePass<MBTileNeiPass>({"nei_pass"}, max_pass->GetVelocityTileOutput(), tile_size);
	}

public:
	inline ~MBTilePass() final = default;
	inline auto GetVelocityTileOutput() {
		return CreateImageAliasOutput({"out"}, GetPass<MBTileNeiPass>({"nei_pass"})->GetVelocityTileOutput());
	}
};

} // namespace rg
