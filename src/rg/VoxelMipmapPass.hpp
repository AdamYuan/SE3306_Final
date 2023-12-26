#pragma once

#include <array>
#include <myvk_rg/RenderGraph.hpp>

namespace rg {

class VoxelMipmapSubpass0 final : public myvk_rg::ComputePassBase {
	MYVK_RG_FRIENDS

private:
	myvk::Ptr<myvk::ComputePipeline> m_pipeline;
	uint32_t m_dst_resolution{};

	void Initialize(uint32_t resolution, myvk_rg::ImageInput voxel) {
		const auto &device = GetRenderGraphPtr()->GetDevicePtr();

		m_dst_resolution = std::max(resolution >> 1u, 1u);
		AddDescriptorInput<0, myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT>(
		    {"voxel_in"}, voxel,
		    myvk::Sampler::Create(device, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE));

		std::vector<myvk_rg::ImageDescriptorInput> descriptor_inputs(6);
		for (uint32_t i = 0; i < 6; ++i) {
			auto mip = CreateResource<myvk_rg::ManagedImage>({"voxel_mip", i}, VK_FORMAT_R16G16B16A16_SFLOAT,
			                                                 VK_IMAGE_VIEW_TYPE_3D);
			mip->SetSize3D({m_dst_resolution, m_dst_resolution, m_dst_resolution}, 0, 1);
			descriptor_inputs[i] = {.input_key = {"mip_in", i}, .resource = mip};
		}
		AddDescriptorInput<1, myvk_rg::Usage::kStorageImageW, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT>(
		    descriptor_inputs);
	}

public:
	inline ~VoxelMipmapSubpass0() final = default;
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline std::array<myvk_rg::ImageOutput, 6> GetVoxelMipmapOutputs() {
		std::array<myvk_rg::ImageOutput, 6> ret{};
		for (uint32_t i = 0; i < 6; ++i)
			ret[i] = MakeImageOutput({"mip_in", i});
		return ret;
	}
};

class VoxelMipmapSubpass final : public myvk_rg::ComputePassBase {
	MYVK_RG_FRIENDS
public:
	myvk::Ptr<myvk::ComputePipeline> m_pipeline;
	uint32_t m_dst_resolution{};

	void Initialize(uint32_t resolution, uint32_t level, std::array<myvk_rg::ImageInput, 6> src_voxels) {
		const auto &device = GetRenderGraphPtr()->GetDevicePtr();
		auto sampler = myvk::Sampler::Create(device, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
		auto mip_resolution = std::max(resolution >> 1u, 1u);
		m_dst_resolution = std::max(mip_resolution >> level, 1u);

		std::vector<myvk_rg::SamplerDescriptorInput> src_descriptor_inputs(6);
		std::vector<myvk_rg::ImageDescriptorInput> descriptor_inputs(6);
		for (uint32_t i = 0; i < 6; ++i) {
			src_descriptor_inputs[i] = {
			    .input_key = {"src_mip_in", i}, .resource = src_voxels[i], .sampler = sampler};

			auto mip = CreateResource<myvk_rg::ManagedImage>({"mip", i}, VK_FORMAT_R16G16B16A16_SFLOAT,
			                                                 VK_IMAGE_VIEW_TYPE_3D);
			mip->SetSize3D({mip_resolution, mip_resolution, mip_resolution}, level, 1);
			descriptor_inputs[i] = {.input_key = {"mip_in", i}, .resource = mip};
		}
		AddDescriptorInput<0, myvk_rg::Usage::kSampledImage, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT>(
		    src_descriptor_inputs);
		AddDescriptorInput<1, myvk_rg::Usage::kStorageImageW, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT>(
		    descriptor_inputs);
	}

public:
	inline ~VoxelMipmapSubpass() final = default;
	void CreatePipeline() final;
	void CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const final;
	inline std::array<myvk_rg::ImageOutput, 6> GetVoxelMipmapOutputs() {
		std::array<myvk_rg::ImageOutput, 6> ret{};
		for (uint32_t i = 0; i < 6; ++i)
			ret[i] = MakeImageOutput({"mip_in", i});
		return ret;
	}
};

class VoxelMipmapPass final : public myvk_rg::PassGroupBase {
	MYVK_RG_FRIENDS

private:
	template <std::size_t N>
	inline std::array<myvk_rg::ImageInput, N> InputsFromOutputs(const std::array<myvk_rg::ImageOutput, N> &outputs) {
		std::array<myvk_rg::ImageInput, N> ret{};
		for (std::size_t i = 0; i < N; ++i)
			ret[i] = outputs[i];
		return ret;
	}
	void Initialize(uint32_t resolution, uint32_t level, myvk_rg::ImageInput src_voxel) {
		assert(level >= 2);
		std::vector<std::array<myvk_rg::ImageOutput, 6>> outputs;
		auto subpass_0 = CreatePass<VoxelMipmapSubpass0>({"subpass", 0}, resolution, src_voxel);
		outputs.push_back(subpass_0->GetVoxelMipmapOutputs());
		for (uint32_t l = 1; l + 1 < level; ++l) {
			auto subpass_l =
			    CreatePass<VoxelMipmapSubpass>({"subpass", l}, resolution, l, InputsFromOutputs(outputs.back()));
			outputs.push_back(subpass_l->GetVoxelMipmapOutputs());
		}

		for (uint32_t i = 0; i < 6; ++i) {
			std::vector<myvk_rg::ImageOutput> aniso_outputs;
			aniso_outputs.reserve(level - 1);
			for (const auto &o : outputs)
				aniso_outputs.push_back(o[i]);
			CreateResourceForce<myvk_rg::CombinedImage>({"voxel_mip", i}, VK_IMAGE_VIEW_TYPE_3D,
			                                            std::move(aniso_outputs));
		}
	}

public:
	inline ~VoxelMipmapPass() final = default;
	inline std::array<const myvk_rg::ImageBase *, 6> GetVoxelMipmapOutputs() {
		std::array<const myvk_rg::ImageBase *, 6> ret{};
		for (uint32_t i = 0; i < 6; ++i)
			ret[i] = GetResource<myvk_rg::CombinedImage>({"voxel_mip", i});
		return ret;
	}
};

} // namespace rg
