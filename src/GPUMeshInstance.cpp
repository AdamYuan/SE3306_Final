#include "GPUMeshInstance.hpp"

GPUMeshInstance::GPUMeshInstance(const myvk::Ptr<GPUMesh> &mesh_ptr, uint32_t max_instance_count)
    : m_mesh_ptr{mesh_ptr}, m_max_instance_count{max_instance_count} {
	const auto &device = m_mesh_ptr->GetDevicePtr();
	m_instance_buffer =
	    myvk::Buffer::Create(device, m_max_instance_count * sizeof(InstanceInfo),
	                         VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
	                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	m_p_instance_info = (InstanceInfo *)m_instance_buffer->GetMappedData();
}

void GPUMeshInstance::SetTransform(TransformSet &&transforms) {
	uint32_t counter = 0;
	for (const auto &it : transforms) {
		if (counter >= m_max_instance_count)
			break;

		auto id = it.first;
		const auto &trans = it.second;
		InstanceInfo ins = {.color = trans.color, .model = trans.model};
		auto prev_it = m_prev_set.find(id);
		ins.prev_model = prev_it == m_prev_set.end() ? trans.model : prev_it->second.model;

		m_p_instance_info[counter++] = ins;
	}
	m_prev_set = std::move(transforms);
}

void GPUMeshInstance::CmdDraw(const myvk::Ptr<myvk::CommandBuffer> &command_buffer, int lod) {
	// Bind two vertex buffers
	if (m_prev_set.empty())
		return;
	VkBuffer handles[] = {m_mesh_ptr->GetVertexBuffer()->GetHandle(), m_instance_buffer->GetHandle()};
	VkDeviceSize offsets[] = {0, 0};
	vkCmdBindVertexBuffers(command_buffer->GetHandle(), 0, 2, handles, offsets);

	command_buffer->CmdBindIndexBuffer(m_mesh_ptr->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

	const auto &lod_info = m_mesh_ptr->GetLodInfo(lod);
	command_buffer->CmdDrawIndexed(lod_info.index_count, m_prev_set.size(), lod_info.first_index,
	                               lod_info.vertex_offset, 0);
}
