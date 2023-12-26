#include "VoxelMipmapPass.hpp"

namespace rg {

inline const auto get_group_size = [](auto x) { return x / 4 + (x % 4 != 0); };

void VoxelMipmapSubpass0::CreatePipeline() {
	const auto &device = GetRenderGraphPtr()->GetDevicePtr();

	auto pipeline_layout = myvk::PipelineLayout::Create(device, {GetVkDescriptorSetLayout()}, {});
	constexpr uint32_t kCompSpv[] = {
#include <shader/voxel_mipmap_0.comp.u32>
	};

	std::shared_ptr<myvk::ShaderModule> shader_module;
	shader_module = myvk::ShaderModule::Create(device, kCompSpv, sizeof(kCompSpv));

	m_pipeline = myvk::ComputePipeline::Create(pipeline_layout, shader_module);
}
void VoxelMipmapSubpass0::CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const {
	command_buffer->CmdBindPipeline(m_pipeline);
	command_buffer->CmdBindDescriptorSets({GetVkDescriptorSet()}, m_pipeline);
	uint32_t group_size = get_group_size(m_dst_resolution);
	command_buffer->CmdDispatch(group_size, group_size, group_size);
}

void VoxelMipmapSubpass::CreatePipeline() {
	const auto &device = GetRenderGraphPtr()->GetDevicePtr();

	auto pipeline_layout = myvk::PipelineLayout::Create(device, {GetVkDescriptorSetLayout()}, {});
	constexpr uint32_t kCompSpv[] = {
#include <shader/voxel_mipmap.comp.u32>
	};

	std::shared_ptr<myvk::ShaderModule> shader_module;
	shader_module = myvk::ShaderModule::Create(device, kCompSpv, sizeof(kCompSpv));

	m_pipeline = myvk::ComputePipeline::Create(pipeline_layout, shader_module);
}
void VoxelMipmapSubpass::CmdExecute(const myvk::Ptr<myvk::CommandBuffer> &command_buffer) const {
	command_buffer->CmdBindPipeline(m_pipeline);
	command_buffer->CmdBindDescriptorSets({GetVkDescriptorSet()}, m_pipeline);
	uint32_t group_size = get_group_size(m_dst_resolution), group_size_6 = get_group_size(m_dst_resolution * 6);
	command_buffer->CmdDispatch(group_size, group_size, group_size_6);
}

} // namespace rg
