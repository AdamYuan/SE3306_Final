#pragma once

#include "../GPUAnimation.hpp"
#include <myvk_rg/RenderGraph.hpp>

namespace rg {

class VoxelClearPass final : public myvk_rg::TransferPassBase {
	MYVK_RG_FRIENDS

private:
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;

	void Initialize(uint32_t resolution) {
		auto voxel =
		    CreateResource<myvk_rg::ManagedImage>({"voxel"}, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_VIEW_TYPE_3D);
		voxel->SetSize3D({resolution, resolution, resolution});
		AddInput<myvk_rg::Usage::kTransferImageDst, VK_PIPELINE_STAGE_2_CLEAR_BIT>({"voxel_in"}, voxel);
	}

public:
	~VoxelClearPass() final;
	inline void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final {
		command_buffer->CmdClearColorImage(GetImageResource({"voxel"})->GetVkImageView()->GetImagePtr(),
		                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, {});
	}
	inline auto GetVoxelOutput() { return MakeImageOutput({"voxel_in"}); }
};

class VoxelDrawPass final : public myvk_rg::GraphicsPassBase {
	MYVK_RG_FRIENDS

private:
	GPUAInstance m_ani_instance;
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;
	uint32_t m_resolution;

	void Initialize(const GPUAInstance &ani_instance, uint32_t resolution, myvk_rg::ImageInput cleared_voxel,
	                myvk_rg::ImageInput shadow_map) {
		m_ani_instance = ani_instance;
		m_resolution = resolution;
		SetAreaForce(resolution, resolution);
		AddDescriptorInput<0, myvk_rg::Usage::kStorageImageW, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>({"voxel_in"},
		                                                                                               cleared_voxel);
		VkSamplerCreateInfo sampler_info = {};
		sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_info.magFilter = VK_FILTER_LINEAR;
		sampler_info.minFilter = VK_FILTER_LINEAR;
		sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		sampler_info.mipLodBias = 0.0f;
		sampler_info.compareOp = VK_COMPARE_OP_LESS;
		sampler_info.compareEnable = VK_TRUE;
		sampler_info.minLod = 0.0f;
		sampler_info.maxLod = VK_LOD_CLAMP_NONE;
		AddDescriptorInput<1, myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>(
		    {"shadow_map_in"}, shadow_map, myvk::Sampler::Create(GetRenderGraphPtr()->GetDevicePtr(), sampler_info));
	}

public:
	~VoxelDrawPass() final;
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline auto GetVoxelOutput() { return MakeImageOutput({"voxel_in"}); }
};

class VoxelizePass final : public myvk_rg::PassGroupBase {
	MYVK_RG_FRIENDS

private:
	void Initialize(const GPUAInstance &ani_instance, uint32_t resolution, myvk_rg::ImageInput shadow_map) {
		auto clear_pass = CreatePass<VoxelClearPass>({"clear_pass"}, resolution);
		CreatePass<VoxelDrawPass>({"draw_pass"}, ani_instance, resolution, clear_pass->GetVoxelOutput(), shadow_map);
	}

public:
	~VoxelizePass() final;
	inline auto GetVoxelOutput() {
		return CreateImageAliasOutput({"voxel"}, GetPass<VoxelDrawPass>({"draw_pass"})->GetVoxelOutput());
	}
};

} // namespace rg
