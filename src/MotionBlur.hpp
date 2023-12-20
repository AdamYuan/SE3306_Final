#pragma once

#include <mygl3/framebuffer.hpp>
#include <mygl3/shader.hpp>
#include <mygl3/texture.hpp>

class MotionBlur {
private:
	mygl3::Shader m_tile_shader;

	mygl3::Texture2D m_depth, m_albedo, m_normal, m_velocity;
	mygl3::FrameBuffer m_fbo;
	int m_width{-1}, m_height{-1};

	void initialize_target(int width, int height);

public:
};
