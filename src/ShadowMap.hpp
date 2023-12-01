#pragma once

#include <mygl3/framebuffer.hpp>
#include <mygl3/texture.hpp>

class ShadowMap {
private:
	mygl3::Texture2D m_depth;
	mygl3::FrameBuffer m_fbo;
	int m_width{-1}, m_height{-1};

	void initialize(int width, int height);

public:
	void UseFBO(int width, int height) {
		initialize(width, height);
		m_fbo.Bind();
	}
	inline void BindOutput(GLuint bind) { m_depth.Bind(bind); }
};
