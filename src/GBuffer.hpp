#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mygl3/framebuffer.hpp>
#include <mygl3/shader.hpp>
#include <mygl3/texture.hpp>

class GBuffer {
private:
	mygl3::Shader m_shader;

	mygl3::Texture2D m_depth, m_albedo, m_normal, m_velocity;
	mygl3::FrameBuffer m_fbo;
	int m_width{-1}, m_height{-1};

	void initialize_target(int width, int height);

public:
	void Initialize();
	template <typename DrawFunc> void Generate(int width, int height, const glm::vec2 &jitter, DrawFunc &&draw_func) {
		initialize_target(width, height);
		m_fbo.Bind();
		m_shader.SetVec2(0, glm::value_ptr(jitter));
		m_shader.Use();
		draw_func();
	}
};
