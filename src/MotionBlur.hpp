#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <mygl3/framebuffer.hpp>
#include <mygl3/shader.hpp>
#include <mygl3/texture.hpp>

class MotionBlur {
private:
	mygl3::Shader m_tile_max_shader, m_tile_nei_shader, m_speed_depth_shader, m_blur_shader;

	mygl3::Texture2D m_tile_0, m_tile, m_speed_depth, m_blur;
	mygl3::FrameBuffer m_tile_nei_fbo, m_speed_depth_fbo, m_blur_fbo;
	int m_width{-1}, m_height{-1}, m_tile_size{-1};

	void initialize_target(int width, int height, int tile_size);
	void initialize_tile_max_shader();

	inline static constexpr auto div_ceil(auto x, auto y) { return x / y + (x % y == 0 ? 0 : 1); }

public:
	void Initialize(const char *quad_vert_str);

	template <typename QuadDrawFunc>
	void Generate(int width, int height, int tile_size, float search_scale, const glm::vec2 &jitter,
	              QuadDrawFunc &&quad_draw_func) {
		initialize_target(width, height, tile_size);
		int tw = div_ceil(m_width, m_tile_size), th = div_ceil(m_height, m_tile_size);
		m_tile_max_shader.Use();
		glDispatchCompute(tw, th, 1);
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

		m_tile_nei_fbo.Bind();
		m_tile_nei_shader.Use();
		quad_draw_func(tw, th);

		// Gather pixel speed & depth information to a texture
		// For optimization
		m_speed_depth_fbo.Bind();
		m_speed_depth_shader.Use();
		m_speed_depth_shader.SetVec2(0, glm::value_ptr(jitter));
		quad_draw_func(width, height);

		m_blur_fbo.Bind();
		m_blur_shader.Use();
		m_blur_shader.SetFloat(0, search_scale);
		quad_draw_func(width, height);
	}
};
