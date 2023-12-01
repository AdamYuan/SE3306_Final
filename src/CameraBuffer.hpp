#pragma once

#include <glm/glm.hpp>
#include <mygl3/buffer.hpp>

class CameraBuffer {
private:
	mygl3::Buffer m_buffer;
	glm::mat4 m_view_proj;

public:
	void Initialize();
	void Update(const glm::mat4 &view_proj);
	void BindUniform(GLuint index) const;
};
