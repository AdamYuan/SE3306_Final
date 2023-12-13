#pragma once

#include <mygl3/framebuffer.hpp>
#include <mygl3/shader.hpp>
#include <mygl3/texture.hpp>

class Bloom {
private:
	mygl3::Shader m_fetch_shader, m_down_shader, m_up_shader;
	mygl3::Texture2D m_bloom;
	std::vector<mygl3::FrameBuffer> m_fbos;
	int m_width{-1}, m_height{-1}, m_mipmap{-1};

	void initialize_target(int width, int height, int mipmap);

public:
	void Initialize();
	template <typename QuadDrawFunc> void Generate(int width, int height, int mipmap, QuadDrawFunc &&quad_draw_func) {
		initialize_target(width, height, mipmap);
		m_fbos[0].Bind();
		m_fetch_shader.Use();
		quad_draw_func(width, height);
	}
};
