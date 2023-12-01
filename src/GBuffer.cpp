#include "GBuffer.hpp"

#include <shader/Binding.h>

void GBuffer::Initialize() {
	m_shader.Initialize();
	constexpr const GLuint kVertSPIRV[] = {
#include <shader/gbuffer.vert.u32>
	};
	constexpr const GLuint kFragSPIRV[] = {
#include <shader/gbuffer.frag.u32>
	};
	m_shader.LoadBinary(kVertSPIRV, sizeof(kVertSPIRV), GL_VERTEX_SHADER);
	m_shader.LoadBinary(kFragSPIRV, sizeof(kFragSPIRV), GL_FRAGMENT_SHADER);
	m_shader.Finalize();
}

void GBuffer::initialize_fbo(int width, int height) {
	if (m_width == width && m_height == height)
		return;
	m_width = width;
	m_height = height;

	m_albedo.Initialize();
	m_albedo.Storage(m_width, m_height, GL_RGB8, 1);
	m_albedo.SetSizeFilter(GL_NEAREST, GL_NEAREST);
	m_albedo.SetWrapFilter(GL_CLAMP_TO_BORDER);

	m_normal.Initialize();
	m_normal.Storage(m_width, m_height, GL_RG8_SNORM, 1);
	m_normal.SetSizeFilter(GL_NEAREST, GL_NEAREST);
	m_normal.SetWrapFilter(GL_CLAMP_TO_BORDER);

	m_depth.Initialize();
	m_depth.Storage(m_width, m_height, GL_DEPTH_COMPONENT32, 1);
	m_depth.SetSizeFilter(GL_LINEAR, GL_LINEAR);
	m_depth.SetWrapFilter(GL_CLAMP_TO_EDGE);

	m_fbo.Initialize();
	m_fbo.AttachTexture2D(m_albedo, GL_COLOR_ATTACHMENT0);
	m_fbo.AttachTexture2D(m_normal, GL_COLOR_ATTACHMENT1);
	m_fbo.AttachTexture2D(m_depth, GL_DEPTH_ATTACHMENT);

	GLenum attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glNamedFramebufferDrawBuffers(m_fbo.Get(), 2, attachments);

	m_albedo.Bind(GBUFFER_ALBEDO_TEXTURE);
	m_normal.Bind(GBUFFER_NORMAL_TEXTURE);
	m_depth.Bind(GBUFFER_DEPTH_TEXTURE);
}
