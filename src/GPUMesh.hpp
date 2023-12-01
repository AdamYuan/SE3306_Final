#pragma once

#include "Mesh.hpp"

#include <span>
#include <vector>

#include <mygl3/buffer.hpp>
#include <mygl3/vertexarray.hpp>

class GPUMesh {
private:
	mygl3::VertexArray m_vertex_array;
	mygl3::Buffer m_vertex_buffer, m_index_buffer, m_draw_cmd_buffer, m_model_buffer;
	std::vector<glm::mat4> m_models;
	bool m_model_changed = false;

public:
	void Initialize(std::span<const Mesh> meshes);
	[[nodiscard]] inline uint32_t GetMeshCount() const { return m_models.size(); }
	inline void SetModel(uint32_t mesh_id, const glm::mat4 &model) {
		m_models[mesh_id] = model;
		m_model_changed = true;
	}
	void Draw();
};
