#include "CameraBuffer.hpp"

#include <glm/gtc/matrix_transform.hpp>

void CameraBuffer::Initialize() {
	m_buffer.Initialize();
	m_buffer.Storage(3 * sizeof(glm::mat4), GL_DYNAMIC_STORAGE_BIT);
}

void CameraBuffer::Update(const glm::mat4 &view_proj, const glm::mat4 &shadow_view_proj) {
	glm::mat4 data[] = {
	    view_proj,
	    glm::inverse(view_proj),
	    shadow_view_proj,
	};
	m_buffer.SubData(0, data, data + 3);
}
void CameraBuffer::BindUniform(GLuint index) const { m_buffer.BindBase(GL_UNIFORM_BUFFER, index); }
