#include "GBuffer.hpp"

#include <shader/Binding.h>

void GBuffer::Initialize() {
	m_shader.Initialize();
	constexpr const char *kVert =
#include <shader/gbuffer.vert.str>
	    ;
	constexpr const char *kFrag =
#include <shader/gbuffer.frag.str>
	    ;
	m_shader.Load(kVert, GL_VERTEX_SHADER);
	m_shader.Load(kFrag, GL_FRAGMENT_SHADER);
	m_shader.Finalize();
}

void GBuffer::initialize_target(int width, int height) {
	if (m_width == width && m_height == height)
		return;
	m_width = width;
	m_height = height;

	m_albedo.Initialize();
	m_albedo.Storage(m_width, m_height, GL_RGB16F, 1);
	m_albedo.SetSizeFilter(GL_LINEAR, GL_LINEAR);
	m_albedo.SetWrapFilter(GL_CLAMP_TO_BORDER);

	m_normal.Initialize();
	m_normal.Storage(m_width, m_height, GL_RG16_SNORM, 1);
	m_normal.SetSizeFilter(GL_LINEAR, GL_LINEAR);
	m_normal.SetWrapFilter(GL_CLAMP_TO_BORDER);

	m_depth.Initialize();
	m_depth.Storage(m_width, m_height, GL_DEPTH_COMPONENT32, 1);
	m_depth.SetSizeFilter(GL_LINEAR, GL_LINEAR);
	m_depth.SetWrapFilter(GL_CLAMP_TO_EDGE);

	m_prev_uv.Initialize();
	m_prev_uv.Storage(m_width, m_height, GL_RG16, 1);
	m_prev_uv.SetSizeFilter(GL_LINEAR, GL_LINEAR);
	m_prev_uv.SetWrapFilter(GL_CLAMP_TO_EDGE);

	m_fbo.Initialize();
	m_fbo.AttachTexture2D(m_albedo, GL_COLOR_ATTACHMENT0);
	m_fbo.AttachTexture2D(m_normal, GL_COLOR_ATTACHMENT1);
	m_fbo.AttachTexture2D(m_prev_uv, GL_COLOR_ATTACHMENT2);
	m_fbo.AttachTexture2D(m_depth, GL_DEPTH_ATTACHMENT);

	GLenum attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	glNamedFramebufferDrawBuffers(m_fbo.Get(), 3, attachments);

	m_albedo.Bind(GBUFFER_ALBEDO_TEXTURE);
	m_normal.Bind(GBUFFER_NORMAL_TEXTURE);
	m_prev_uv.Bind(GBUFFER_PREV_UV_TEXTURE);
	m_depth.Bind(GBUFFER_DEPTH_TEXTURE);
}
