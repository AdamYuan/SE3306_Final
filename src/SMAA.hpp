#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mygl3/framebuffer.hpp>
#include <mygl3/shader.hpp>
#include <mygl3/texture.hpp>
#include <shader/Binding.h>

class SMAA {
private:
	mygl3::Shader m_shader;
	mygl3::Texture2D m_texture;
	mygl3::FrameBuffer m_fbo;
	int m_width{-1}, m_height{-1};

	void initialize_target(int width, int height);

public:
	void Initialize(const char *quad_vert_str);
	template <typename QuadDrawFunc>
	void Generate(int width, int height, QuadDrawFunc &&quad_draw_func) {
		initialize_target(width, height);

		m_fbo.Bind();
		m_shader.Use();

		quad_draw_func();
	}
};
