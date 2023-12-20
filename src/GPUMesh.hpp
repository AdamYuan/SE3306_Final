#pragma once

#include "Mesh.hpp"

#include <span>
#include <vector>

#include <mygl3/buffer.hpp>
#include <mygl3/vertexarray.hpp>

class GPUMesh {
private:
	mygl3::VertexArray m_vertex_array;
	mygl3::Buffer m_vertex_buffer, m_index_buffer, m_instance_info_buffer;
	struct InstanceInfo {
		glm::vec4 color;
		glm::mat4 model;
	};
	std::vector<InstanceInfo> m_instance_infos;
	struct LodInfo {
		GLsizei count, base_vertex;
		void *base_index;
	};
	std::vector<LodInfo> m_lods;
	uint32_t m_instance_count = 0;
	bool m_changed = false;

public:
	void Initialize(std::span<const Mesh> mesh_lods, uint32_t max_instance_count = 1);
	inline uint32_t GetMaxMeshCount() const { return m_instance_infos.size(); }
	inline uint32_t GetCurrentMeshCount() const { return m_instance_count; }
	inline void SetInstanceCount(uint32_t instance_count) {
		if (instance_count == m_instance_count)
			return;
		m_instance_count = glm::clamp(instance_count, 0u, GetMaxMeshCount());
		m_changed = true;
	}
	inline void SetModel(uint32_t mesh_id, const glm::mat4 &model) {
		m_instance_infos[mesh_id].model = model;
		m_changed = true;
	}
	inline void SetColor(uint32_t mesh_id, const glm::vec4 &color) {
		m_instance_infos[mesh_id].color = color;
		m_changed = true;
	}
	void Draw(uint32_t lod = 0);
};
