#pragma once

#include <mygl3/framebuffer.hpp>
#include <mygl3/shader.hpp>
#include <mygl3/texture.hpp>

class LightPass {
private:
	mygl3::Shader m_shader;
	mygl3::Texture2D m_light;
	mygl3::FrameBuffer m_fbo;
	int m_width{-1}, m_height{-1};

	GLuint m_tick = 0;

	void initialize_target(int width, int height);

public:
	void Initialize(const char *quad_vert_str);

	template <typename QuadDrawFunc> void Generate(int width, int height, QuadDrawFunc &&quad_draw_func) {
		initialize_target(width, height);
		m_fbo.Bind();
		m_shader.Use();
		m_shader.SetUint(0, m_tick++);
		quad_draw_func();
	}
};
