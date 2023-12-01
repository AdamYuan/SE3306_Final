#pragma once

#include <mygl3/framebuffer.hpp>
#include <mygl3/shader.hpp>
#include <mygl3/texture.hpp>

class ShadowMap {
private:
	mygl3::Shader m_shader;

	mygl3::Texture2D m_depth;
	mygl3::FrameBuffer m_fbo;
	int m_width{-1}, m_height{-1};

	void initialize_fbo(int width, int height);

public:
	void Initialize();
	template <typename DrawFunc> void Generate(int width, int height, DrawFunc &&draw_func) {
		initialize_fbo(width, height);
		m_fbo.Bind();
		m_shader.Use();
		draw_func();
	}
};
