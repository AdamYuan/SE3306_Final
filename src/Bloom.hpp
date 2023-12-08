#pragma once

#include <mygl3/framebuffer.hpp>
#include <mygl3/shader.hpp>
#include <mygl3/texture.hpp>

class Bloom {
private:
	mygl3::Shader m_shader_0, m_shader_1;
	mygl3::Texture2D m_bloom_0, m_bloom_1;
	mygl3::FrameBuffer m_fbo_0, m_fbo_1;
	int m_width{-1}, m_height{-1};

	void initialize_target(int width, int height);

public:
	void Initialize();
	template <typename QuadDrawFunc> void Generate(int width, int height, QuadDrawFunc &&quad_draw_func) {
		initialize_target(width, height);
		m_fbo_0.Bind();
		m_shader_0.Use();
		quad_draw_func();
		m_fbo_1.Bind();
		m_shader_1.Use();
		quad_draw_func();
	}
};
