#include "LightPass.hpp"

#include <shader/Binding.h>

void LightPass::Initialize(const char *quad_vert_str) {
	m_shader.Initialize();
	constexpr const char *kFrag =
#include <shader/light.frag.str>
	    ;
	m_shader.Load(quad_vert_str, GL_VERTEX_SHADER);
	m_shader.Load(kFrag, GL_FRAGMENT_SHADER);
	m_shader.Finalize();
}

void LightPass::initialize_target(int width, int height) {
	if (m_width == width && m_height == height)
		return;
	m_width = width;
	m_height = height;

	m_light.Initialize();
	m_light.Storage(m_width, m_height, GL_RGB16, 1);
	m_light.SetSizeFilter(GL_NEAREST, GL_NEAREST);
	m_light.SetWrapFilter(GL_CLAMP_TO_EDGE);
	m_light.Bind(LIGHT_TEXTURE);

	m_fbo.Initialize();
	m_fbo.AttachTexture2D(m_light, GL_COLOR_ATTACHMENT0);
	GLenum attachments[] = {GL_COLOR_ATTACHMENT0};
	glNamedFramebufferDrawBuffers(m_fbo.Get(), 1, attachments);
}
