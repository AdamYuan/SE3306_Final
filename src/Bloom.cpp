#include "Bloom.hpp"

#include <shader/Binding.h>

void Bloom::Initialize() {
	constexpr const char *kQuadVert =
#include <shader/quad.vert.str>
	    ;
	{
		m_shader_0.Initialize();
		constexpr const char *kFrag =
#include <shader/bloom_0.frag.str>
		    ;
		m_shader_0.Load(kQuadVert, GL_VERTEX_SHADER);
		m_shader_0.Load(kFrag, GL_FRAGMENT_SHADER);
		m_shader_0.Finalize();
	}
	{
		m_shader_1.Initialize();
		constexpr const char *kFrag =
#include <shader/bloom_1.frag.str>
		    ;
		m_shader_1.Load(kQuadVert, GL_VERTEX_SHADER);
		m_shader_1.Load(kFrag, GL_FRAGMENT_SHADER);
		m_shader_1.Finalize();
	}
}

void Bloom::initialize_target(int width, int height) {
	if (m_width == width && m_height == height)
		return;
	m_width = width;
	m_height = height;

	m_bloom_0.Initialize();
	m_bloom_0.Storage(m_width, m_height, GL_RGBA16F, 1);
	m_bloom_0.SetSizeFilter(GL_LINEAR, GL_LINEAR);
	m_bloom_0.SetWrapFilter(GL_CLAMP_TO_EDGE);

	m_bloom_1.Initialize();
	m_bloom_1.Storage(m_width, m_height, GL_RGBA16F, 1);
	m_bloom_1.SetSizeFilter(GL_LINEAR, GL_LINEAR);
	m_bloom_1.SetWrapFilter(GL_CLAMP_TO_EDGE);

	m_fbo_0.Initialize();
	m_fbo_0.AttachTexture2D(m_bloom_0, GL_COLOR_ATTACHMENT0);
	m_fbo_1.Initialize();
	m_fbo_1.AttachTexture2D(m_bloom_1, GL_COLOR_ATTACHMENT0);

	GLenum attachments[] = {GL_COLOR_ATTACHMENT0};
	glNamedFramebufferDrawBuffers(m_fbo_0.Get(), 1, attachments);
	glNamedFramebufferDrawBuffers(m_fbo_1.Get(), 1, attachments);

	m_bloom_0.Bind(BLOOM_0_TEXTURE);
	m_bloom_1.Bind(BLOOM_TEXTURE);
}
