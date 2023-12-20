#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <mygl3/framebuffer.hpp>
#include <mygl3/shader.hpp>

class ScreenPass {
private:
	mygl3::Shader m_shader;

public:
	void Initialize(const char *quad_vert_str);

	template <typename QuadDrawFunc> void Generate(const glm::vec2 &jitter, QuadDrawFunc &&quad_draw_func) {
		mygl3::FrameBuffer::Unbind();
		m_shader.Use();
		m_shader.SetVec2(0, glm::value_ptr(jitter));
		quad_draw_func();
	}
};
