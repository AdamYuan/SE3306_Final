#pragma once

#include "../GPUAnimation.hpp"
#include <myvk_rg/RenderGraph.hpp>

namespace rg {

class VoxelClearPass final : public myvk_rg::TransferPassBase {
private:
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;

public:
	VoxelClearPass(myvk_rg::Parent parent, uint32_t resolution) : TransferPassBase(parent) {
		auto voxel =
		    CreateResource<myvk_rg::ManagedImage>({"voxel"}, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_VIEW_TYPE_3D);
		voxel->SetSize3D({resolution, resolution, resolution});
		AddInput<myvk_rg::Usage::kTransferImageDst, VK_PIPELINE_STAGE_2_CLEAR_BIT>({"voxel_in"}, voxel->Alias());
	}

	inline ~VoxelClearPass() final = default;
	inline void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final {
		command_buffer->CmdClearColorImage(GetImageResource({"voxel"})->GetVkImageView()->GetImagePtr(),
		                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, {});
	}
	inline auto GetVoxelOutput() { return MakeImageOutput({"voxel_in"}); }
};

class VoxelDrawPass final : public myvk_rg::GraphicsPassBase {
private:
	GPUAInstance m_ani_instance;
	myvk::Ptr<myvk::GraphicsPipeline> m_pipeline;
	uint32_t m_resolution{};

public:
	VoxelDrawPass(myvk_rg::Parent parent, const GPUAInstance &ani_instance, uint32_t resolution,
	              const myvk_rg::Image &cleared_voxel, const myvk_rg::Image &shadow_map)
	    : GraphicsPassBase(parent) {
		m_ani_instance = ani_instance;
		m_resolution = resolution;
		SetRenderArea(VkExtent2D{resolution, resolution});
		AddDescriptorInput<myvk_rg::Usage::kStorageImageW, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>({0}, {"voxel_in"},
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
		AddDescriptorInput<myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT>(
		    {1}, {"shadow_map_in"}, shadow_map,
		    myvk::Sampler::Create(GetRenderGraphPtr()->GetDevicePtr(), sampler_info));
	}

	inline ~VoxelDrawPass() final = default;
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline auto GetVoxelOutput() { return MakeImageOutput({"voxel_in"}); }
};

class VoxelizePass final : public myvk_rg::PassGroupBase {
public:
	VoxelizePass(myvk_rg::Parent parent, const GPUAInstance &ani_instance, uint32_t resolution,
	             const myvk_rg::Image &shadow_map)
	    : PassGroupBase(parent) {
		auto clear_pass = CreatePass<VoxelClearPass>({"clear_pass"}, resolution);
		CreatePass<VoxelDrawPass>({"draw_pass"}, ani_instance, resolution, clear_pass->GetVoxelOutput(), shadow_map);
	}

	inline ~VoxelizePass() final = default;
	inline auto GetVoxelOutput() { return GetPass<VoxelDrawPass>({"draw_pass"})->GetVoxelOutput(); }
};

} // namespace rg
