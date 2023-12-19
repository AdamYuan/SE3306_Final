#pragma once

#include <mygl3/framebuffer.hpp>
#include <mygl3/shader.hpp>
#include <mygl3/texture.hpp>

class Bloom {
private:
	mygl3::Shader m_down_0_shader, m_down_shader, m_up_shader;
	mygl3::Texture2D m_bloom;
	std::vector<mygl3::FrameBuffer> m_fbos;
	int m_width{-1}, m_height{-1}, m_mipmap{-1};

	void initialize_target(int width, int height, int mipmap);

public:
	void Initialize(const char *quad_vert_str);
	template <typename QuadDrawFunc>
	void Generate(int width, int height, int mipmap, float filter_radius, QuadDrawFunc &&quad_draw_func) {
		initialize_target(width, height, mipmap);

		m_fbos[1].Bind();
		m_down_0_shader.Use();
		quad_draw_func(width >> 1, height >> 1);

		m_down_shader.Use();
		for (int m = 2; m < mipmap; ++m) {
			m_fbos[m].Bind();
			m_down_shader.SetInt(0, m - 1);
			quad_draw_func(width >> m, height >> m);
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendEquation(GL_FUNC_ADD);
		m_up_shader.Use();
		m_up_shader.SetFloat(1, filter_radius);
		for (int m = mipmap - 2; m >= 0; --m) {
			if (m == 0)
				glDisable(GL_BLEND);
			m_fbos[m].Bind();
			m_up_shader.SetInt(0, m + 1);
			quad_draw_func(width >> m, height >> m);
		}
	}
};
