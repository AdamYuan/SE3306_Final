#include "CameraBuffer.hpp"

#include <glm/gtc/matrix_transform.hpp>

void CameraBuffer::Initialize() {
	m_buffer.Initialize();
	m_buffer.Storage(2 * sizeof(glm::mat4), GL_DYNAMIC_STORAGE_BIT);
	Update(glm::identity<glm::mat4>());
}

void CameraBuffer::Update(const glm::mat4 &view_proj) {
	if (view_proj == m_view_proj)
		return;
	m_view_proj = view_proj;
	glm::mat4 data[] = {
	    view_proj,
	    glm::inverse(view_proj),
	};
	m_buffer.SubData(0, data, data + 2);
}
void CameraBuffer::BindUniform(GLuint index) const {
	m_buffer.BindBase(GL_UNIFORM_BUFFER, index);
}
