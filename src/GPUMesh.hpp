#pragma once

#include "Mesh.hpp"
#include <myvk/Buffer.hpp>
#include <myvk/CommandPool.hpp>
#include <span>
#include <vector>

class GPUMesh final : public myvk::DeviceObjectBase {
private:
	myvk::Ptr<myvk::Buffer> m_vertex_buffer, m_index_buffer;
	struct LodInfo {
		uint32_t index_count, first_index;
		int32_t vertex_offset;
	};
	std::vector<LodInfo> m_lods;

public:
	explicit GPUMesh(const myvk::Ptr<myvk::CommandPool> &command_pool, std::span<const Mesh> mesh_lods);
	inline ~GPUMesh() final = default;

	inline const myvk::Ptr<myvk::Device> &GetDevicePtr() const final { return m_vertex_buffer->GetDevicePtr(); }

	inline const auto &GetLodInfo(int lod) const { return m_lods[lod]; }
	inline const auto &GetVertexBuffer() const { return m_vertex_buffer; }
	inline const auto &GetIndexBuffer() const { return m_index_buffer; }
};
