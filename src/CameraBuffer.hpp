#pragma once

#include <glm/glm.hpp>
#include <mygl3/buffer.hpp>

class CameraBuffer {
private:
	mygl3::Buffer m_buffer;

public:
	void Initialize();
	void Update(const glm::mat4 &view_proj, const glm::mat4 &inv_view_proj, const glm::mat4 &shadow_view_proj);
};
