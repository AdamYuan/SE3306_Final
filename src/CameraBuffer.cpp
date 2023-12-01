#include "CameraBuffer.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <shader/Binding.h>

void CameraBuffer::Initialize() {
	m_buffer.Initialize();
	m_buffer.Storage(3 * sizeof(glm::mat4), GL_DYNAMIC_STORAGE_BIT);
	m_buffer.BindBase(GL_UNIFORM_BUFFER, CAMERA_UNIFORM_BUFFER);
}

void CameraBuffer::Update(const glm::mat4 &view_proj, const glm::mat4 &shadow_view_proj) {
	glm::mat4 data[] = {
	    view_proj,
	    glm::inverse(view_proj),
	    shadow_view_proj,
	};
	m_buffer.SubData(0, data, data + 3);
}
