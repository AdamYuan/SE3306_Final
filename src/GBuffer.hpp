#pragma once

#include <mygl3/framebuffer.hpp>
#include <mygl3/texture.hpp>

class GBuffer {
private:
	mygl3::Texture2D m_depth, m_albedo, m_normal;
	mygl3::FrameBuffer m_fbo;
	int m_width{-1}, m_height{-1};

	void initialize(int width, int height);

public:
	void UseFBO(int width, int height) {
		initialize(width, height);
		m_fbo.Bind();
	}
	inline void BindAlbedoOutput(GLuint bind) { m_albedo.Bind(bind); }
	inline void BindNormalOutput(GLuint bind) { m_normal.Bind(bind); }
	inline void BindDepthOutput(GLuint bind) { m_depth.Bind(bind); }
};
