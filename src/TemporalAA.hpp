#pragma once

#include <glm/glm.hpp>
#include <mygl3/framebuffer.hpp>
#include <mygl3/shader.hpp>
#include <mygl3/texture.hpp>
#include <shader/Binding.h>

class TemporalAA {
private:
	mygl3::Shader m_shader;
	mygl3::Texture2D m_textures[2];
	mygl3::FrameBuffer m_fbos[2];
	int m_width{-1}, m_height{-1};
	uint64_t m_tick{0};

	void initialize_target(int width, int height);

public:
	void Initialize(const char *quad_vert_str);
	glm::vec2 GetJitter(int width, int height);
	template <typename QuadDrawFunc> void Generate(int width, int height, QuadDrawFunc &&quad_draw_func) {
		initialize_target(width, height);

		bool cur = m_tick & 1u, prev = !cur;
		m_textures[prev].Bind(TAA_TEXTURE);
		m_fbos[cur].Bind();
		m_shader.Use();
		m_shader.SetInt(0, m_tick == 0);
		quad_draw_func();
		m_textures[cur].Bind(TAA_TEXTURE);

		++m_tick;
	}
};
