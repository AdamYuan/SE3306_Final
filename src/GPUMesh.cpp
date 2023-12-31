#include "GPUMesh.hpp"

void GPUMesh::Initialize(std::span<const Mesh> mesh_lods, uint32_t max_instance_count) {
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	for (const Mesh &mesh : mesh_lods) {
		LodInfo lod = {.count = (GLsizei)mesh.GetTriangles().size() * 3,
		               .base_vertex = (GLsizei)vertices.size(),
		               .base_index = (void *)intptr_t(indices.size() * sizeof(GLuint))};
		m_lods.push_back(lod);

		vertices.insert(vertices.end(), mesh.GetVertices().begin(), mesh.GetVertices().end());
		for (const auto &tri : mesh.GetTriangles()) {
			indices.push_back(tri[0]);
			indices.push_back(tri[1]);
			indices.push_back(tri[2]);
		}
	}

	m_vertex_buffer.Initialize();
	m_vertex_buffer.Storage(vertices.data(), vertices.data() + vertices.size(), 0);
	m_index_buffer.Initialize();
	m_index_buffer.Storage(indices.data(), indices.data() + indices.size(), 0);

	m_instance_count = max_instance_count;

	m_instance_infos.resize(
	    max_instance_count,
	    {.color = {}, .model = glm::identity<glm::mat4>(), .prev_model = glm::identity<glm::mat4>()});
	m_instance_info_buffer.Initialize();
	m_instance_info_buffer.Storage(m_instance_infos.data(), m_instance_infos.data() + m_instance_infos.size(),
	                               GL_DYNAMIC_STORAGE_BIT);

	m_vertex_array.Initialize();
	{
		constexpr GLuint kPos = 0, kNormal = 1, kColor = 2, kInstanceColor = 3, kModel = 4, kPrevModel = 8;
		constexpr GLuint kVbo0 = 0, kVbo1 = 1;
		glEnableVertexArrayAttrib(m_vertex_array.Get(), kPos);
		glVertexArrayAttribFormat(m_vertex_array.Get(), kPos, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(m_vertex_array.Get(), kPos, kVbo0);

		glEnableVertexArrayAttrib(m_vertex_array.Get(), kNormal);
		glVertexArrayAttribFormat(m_vertex_array.Get(), kNormal, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat));
		glVertexArrayAttribBinding(m_vertex_array.Get(), kNormal, kVbo0);

		glEnableVertexArrayAttrib(m_vertex_array.Get(), kColor);
		glVertexArrayAttribFormat(m_vertex_array.Get(), kColor, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat));
		glVertexArrayAttribBinding(m_vertex_array.Get(), kColor, kVbo0);

		glVertexArrayBindingDivisor(m_vertex_array.Get(), kVbo1, 1);

		glEnableVertexArrayAttrib(m_vertex_array.Get(), kInstanceColor);
		glVertexArrayAttribFormat(m_vertex_array.Get(), kInstanceColor, 4, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(m_vertex_array.Get(), kInstanceColor, kVbo1);

		glEnableVertexArrayAttrib(m_vertex_array.Get(), kModel);
		glVertexArrayAttribFormat(m_vertex_array.Get(), kModel, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat));
		glVertexArrayAttribBinding(m_vertex_array.Get(), kModel, kVbo1);
		glEnableVertexArrayAttrib(m_vertex_array.Get(), kModel + 1);
		glVertexArrayAttribFormat(m_vertex_array.Get(), kModel + 1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat));
		glVertexArrayAttribBinding(m_vertex_array.Get(), kModel + 1, kVbo1);
		glEnableVertexArrayAttrib(m_vertex_array.Get(), kModel + 2);
		glVertexArrayAttribFormat(m_vertex_array.Get(), kModel + 2, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat));
		glVertexArrayAttribBinding(m_vertex_array.Get(), kModel + 2, kVbo1);
		glEnableVertexArrayAttrib(m_vertex_array.Get(), kModel + 3);
		glVertexArrayAttribFormat(m_vertex_array.Get(), kModel + 3, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(GLfloat));
		glVertexArrayAttribBinding(m_vertex_array.Get(), kModel + 3, kVbo1);

		glEnableVertexArrayAttrib(m_vertex_array.Get(), kPrevModel);
		glVertexArrayAttribFormat(m_vertex_array.Get(), kPrevModel, 4, GL_FLOAT, GL_FALSE, 20 * sizeof(GLfloat));
		glVertexArrayAttribBinding(m_vertex_array.Get(), kPrevModel, kVbo1);
		glEnableVertexArrayAttrib(m_vertex_array.Get(), kPrevModel + 1);
		glVertexArrayAttribFormat(m_vertex_array.Get(), kPrevModel + 1, 4, GL_FLOAT, GL_FALSE, 24 * sizeof(GLfloat));
		glVertexArrayAttribBinding(m_vertex_array.Get(), kPrevModel + 1, kVbo1);
		glEnableVertexArrayAttrib(m_vertex_array.Get(), kPrevModel + 2);
		glVertexArrayAttribFormat(m_vertex_array.Get(), kPrevModel + 2, 4, GL_FLOAT, GL_FALSE, 28 * sizeof(GLfloat));
		glVertexArrayAttribBinding(m_vertex_array.Get(), kPrevModel + 2, kVbo1);
		glEnableVertexArrayAttrib(m_vertex_array.Get(), kPrevModel + 3);
		glVertexArrayAttribFormat(m_vertex_array.Get(), kPrevModel + 3, 4, GL_FLOAT, GL_FALSE, 32 * sizeof(GLfloat));
		glVertexArrayAttribBinding(m_vertex_array.Get(), kPrevModel + 3, kVbo1);

		GLuint buffers[2] = {m_vertex_buffer.Get(), m_instance_info_buffer.Get()};
		GLintptr offsets[2] = {0, 0};
		GLsizei strides[2] = {3 * sizeof(glm::vec3), sizeof(InstanceInfo)};
		glVertexArrayVertexBuffers(m_vertex_array.Get(), 0, 2, buffers, offsets, strides);

		glVertexArrayElementBuffer(m_vertex_array.Get(), m_index_buffer.Get());
	}
}

void GPUMesh::Draw(uint32_t lod) {
	if (m_instance_count == 0)
		return;
	if (m_changed) {
		m_instance_info_buffer.SubData(0, m_instance_infos.data(), m_instance_infos.data() + m_instance_count);
		m_changed = false;
	}
	m_vertex_array.Bind();
	glDrawElementsInstancedBaseVertex(GL_TRIANGLES, m_lods[lod].count, GL_UNSIGNED_INT, m_lods[lod].base_index,
	                                  (GLsizei)m_instance_count, m_lods[lod].base_vertex);
}
