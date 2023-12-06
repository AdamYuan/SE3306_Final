#pragma once

#include "Mesh.hpp"

#include <span>
#include <vector>

#include <mygl3/buffer.hpp>
#include <mygl3/vertexarray.hpp>

class GPUMesh {
private:
	mygl3::VertexArray m_vertex_array;
	mygl3::Buffer m_vertex_buffer, m_index_buffer, m_draw_cmd_buffer, m_instance_info_buffer;
	struct InstanceInfo {
		glm::vec4 color;
		glm::mat4 model;
	};
	std::vector<InstanceInfo> m_instance_infos;
	uint32_t m_count = 0;
	bool m_changed = false;

public:
	void Initialize(std::span<const Mesh> meshes, std::span<const uint32_t> counts = {});
	inline uint32_t GetMaxMeshCount() const { return m_instance_infos.size(); }
	inline uint32_t GetCurrentMeshCount() const { return m_count; }
	inline void SetMeshCount(uint32_t count) { m_count = glm::clamp(count, 0u, GetMaxMeshCount()); }
	inline void SetModel(uint32_t mesh_id, const glm::mat4 &model) {
		m_instance_infos[mesh_id].model = model;
		m_changed = true;
	}
	inline void SetColor(uint32_t mesh_id, const glm::vec4 &color) {
		m_instance_infos[mesh_id].color = color;
		m_changed = true;
	}
	void Draw();
};
