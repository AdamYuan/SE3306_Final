#pragma once

#include <mygl3/framebuffer.hpp>
#include <mygl3/shader.hpp>
#include <mygl3/texture.hpp>

class MotionBlur {
private:
	mygl3::Shader m_tile_max_shader, m_tile_nei_shader, m_blur_shader;

	mygl3::Texture2D m_tile_0, m_tile, m_blur;
	mygl3::FrameBuffer m_tile_nei_fbo, m_blur_fbo;
	int m_width{-1}, m_height{-1}, m_tile_size{-1};

	void initialize_target(int width, int height, int tile_size);
	void initialize_tile_max_shader();

	void generate_tile_max();

public:
	void Initialize(const char *quad_vert_str);

	template <typename QuadDrawFunc>
	void Generate(int width, int height, int tile_size, QuadDrawFunc &&quad_draw_func) {
		initialize_target(width, height, tile_size);
		generate_tile_max();
	}
};
