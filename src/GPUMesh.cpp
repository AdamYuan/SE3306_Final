#include "GPUMesh.hpp"

struct DrawElementsIndirectCommand {
	GLuint count;
	GLuint instance_count;
	GLuint first_index;
	GLint base_vertex;
	GLuint base_instance;
};

void GPUMesh::Initialize(std::span<const Mesh> meshes) {
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	GLuint instance = 0;
	std::vector<DrawElementsIndirectCommand> draw_commands;
	draw_commands.reserve(meshes.size());
	for (const auto &mesh : meshes) {
		draw_commands.push_back({
		    .count = GLuint(mesh.GetTriangles().size() * 3),
		    .instance_count = 1,
		    .first_index = GLuint(indices.size()),
		    .base_vertex = GLint(vertices.size()),
		    .base_instance = instance,
		});

		vertices.insert(vertices.end(), mesh.GetVertices().begin(), mesh.GetVertices().end());
		for (const auto &tri : mesh.GetTriangles()) {
			indices.push_back(tri[0]);
			indices.push_back(tri[1]);
			indices.push_back(tri[2]);
		}

		++instance;
	}
	m_vertex_buffer.Initialize();
	m_vertex_buffer.Storage(vertices.data(), vertices.data() + vertices.size(), 0);
	m_index_buffer.Initialize();
	m_index_buffer.Storage(indices.data(), indices.data() + indices.size(), 0);
	m_draw_cmd_buffer.Initialize();
	m_draw_cmd_buffer.Storage(draw_commands.data(), draw_commands.data() + draw_commands.size(), 0);

	m_models.resize(meshes.size(), glm::identity<glm::mat4>());
	m_model_buffer.Initialize();
	m_model_buffer.Storage(m_models.data(), m_models.data() + m_models.size(), GL_DYNAMIC_STORAGE_BIT);

	m_vertex_array.Initialize();
	{
		constexpr GLuint kPos = 0, kNormal = 1, kColor = 2, kModel = 3;
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

		glEnableVertexArrayAttrib(m_vertex_array.Get(), kModel);
		glVertexArrayAttribFormat(m_vertex_array.Get(), kModel, 4, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(m_vertex_array.Get(), kModel, kVbo1);
		glEnableVertexArrayAttrib(m_vertex_array.Get(), kModel + 1);
		glVertexArrayAttribFormat(m_vertex_array.Get(), kModel + 1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat));
		glVertexArrayAttribBinding(m_vertex_array.Get(), kModel + 1, kVbo1);
		glEnableVertexArrayAttrib(m_vertex_array.Get(), kModel + 2);
		glVertexArrayAttribFormat(m_vertex_array.Get(), kModel + 2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat));
		glVertexArrayAttribBinding(m_vertex_array.Get(), kModel + 2, kVbo1);
		glEnableVertexArrayAttrib(m_vertex_array.Get(), kModel + 3);
		glVertexArrayAttribFormat(m_vertex_array.Get(), kModel + 3, 4, GL_FLOAT, GL_FALSE, 12 * sizeof(GLfloat));
		glVertexArrayAttribBinding(m_vertex_array.Get(), kModel + 3, kVbo1);

		GLuint buffers[2] = {m_vertex_buffer.Get(), m_model_buffer.Get()};
		GLintptr offsets[2] = {0, 0};
		GLsizei strides[2] = {3 * sizeof(glm::vec3), sizeof(glm::mat4)};
		glVertexArrayVertexBuffers(m_vertex_array.Get(), 0, 2, buffers, offsets, strides);

		glVertexArrayElementBuffer(m_vertex_array.Get(), m_index_buffer.Get());
	}
}

void GPUMesh::Draw() {
	if (m_model_changed) {
		m_model_buffer.SubData(0, m_models.data(), m_models.data() + m_models.size());
		m_model_changed = false;
	}
	m_vertex_array.Bind();
	m_draw_cmd_buffer.Bind(GL_DRAW_INDIRECT_BUFFER);
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, (GLsizei)m_models.size(), 0);
}
