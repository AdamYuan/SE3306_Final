#pragma once

#include <mygl3/buffer.hpp>
#include <mygl3/framebuffer.hpp>
#include <mygl3/texture.hpp>

class SSAOBuffer {
private:
	mygl3::Texture2D m_occlusion, m_noise;
	mygl3::Texture1D m_sample;
	mygl3::FrameBuffer m_fbo;
	int m_width{-1}, m_height{-1};

	void initialize(int width, int height);

public:
	void UseFBO(int width, int height) {
		initialize(width, height);
		m_fbo.Bind();
	}
	inline void BindSampleTexture(GLuint bind) { m_sample.Bind(bind); }
	inline void BindNoiseTexture(GLuint bind) { m_noise.Bind(bind); }
	inline void BindOcclusionOutput(GLuint bind) { m_occlusion.Bind(bind); }
};
