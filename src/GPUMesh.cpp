#include "GPUMesh.hpp"

#include <myvk/CommandBuffer.hpp>

GPUMesh::GPUMesh(const myvk::Ptr<myvk::CommandPool> &command_pool, std::span<const Mesh> mesh_lods) {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	for (const Mesh &mesh : mesh_lods) {
		LodInfo lod = {.index_count = (uint32_t)mesh.GetTriangles().size() * 3,
		               .first_index = (uint32_t)indices.size(),
		               .vertex_offset = (int32_t)vertices.size()};
		m_lods.push_back(lod);

		vertices.insert(vertices.end(), mesh.GetVertices().begin(), mesh.GetVertices().end());
		for (const auto &tri : mesh.GetTriangles()) {
			indices.push_back(tri[0]);
			indices.push_back(tri[1]);
			indices.push_back(tri[2]);
		}
	}
	const auto &device = command_pool->GetDevicePtr();
	m_vertex_buffer = myvk::Buffer::Create(device, vertices.size() * sizeof(Vertex), 0,
	                                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	m_index_buffer = myvk::Buffer::Create(device, indices.size() * sizeof(uint32_t), 0,
	                                      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	auto vertex_staging = myvk::Buffer::CreateStaging(device, vertices.begin(), vertices.end());
	auto index_staging = myvk::Buffer::CreateStaging(device, indices.begin(), indices.end());

	auto command_buffer = myvk::CommandBuffer::Create(command_pool);
	command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	command_buffer->CmdCopy(vertex_staging, m_vertex_buffer, {VkBufferCopy{.size = vertex_staging->GetSize()}});
	command_buffer->CmdCopy(index_staging, m_index_buffer, {VkBufferCopy{.size = index_staging->GetSize()}});
	command_buffer->End();

	auto fence = myvk::Fence::Create(device);
	command_buffer->Submit(fence);
	fence->Wait();
}