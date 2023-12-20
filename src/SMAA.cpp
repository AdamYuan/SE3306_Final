#include "SMAA.hpp"

void SMAA::Initialize(const char *quad_vert_str) {
	m_shader.Initialize();
	constexpr const char *kFrag =
#include <shader/smaa.frag.str>
	    ;
	m_shader.Load(quad_vert_str, GL_VERTEX_SHADER);
	m_shader.Load(kFrag, GL_FRAGMENT_SHADER);
	m_shader.Finalize();
}

void SMAA::initialize_target(int width, int height) {
	if (m_width == width && m_height == height)
		return;
	m_width = width;
	m_height = height;

	GLenum attachments[] = {GL_COLOR_ATTACHMENT0};
	m_texture.Initialize();
	m_texture.Storage(m_width, m_height, GL_RGB16, 1);
	m_texture.SetSizeFilter(GL_LINEAR, GL_LINEAR);
	m_texture.SetWrapFilter(GL_CLAMP_TO_BORDER);
	m_texture.Bind(AA_TEXTURE);

	m_fbo.Initialize();
	m_fbo.AttachTexture2D(m_texture, GL_COLOR_ATTACHMENT0);
	glNamedFramebufferDrawBuffers(m_fbo.Get(), 1, attachments);
}
