#pragma once

#include <mygl3/framebuffer.hpp>
#include <mygl3/shader.hpp>
#include <mygl3/texture.hpp>

class GBuffer {
private:
	mygl3::Shader m_shader;

	mygl3::Texture2D m_depth, m_albedo, m_normal, m_prev_uv;
	mygl3::FrameBuffer m_fbo;
	int m_width{-1}, m_height{-1};

	void initialize_target(int width, int height);

public:
	void Initialize();
	template <typename DrawFunc> void Generate(int width, int height, DrawFunc &&draw_func) {
		initialize_target(width, height);
		m_fbo.Bind();
		m_shader.Use();
		draw_func();
	}
};
