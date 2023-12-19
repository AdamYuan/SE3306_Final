#include "Bloom.hpp"

#include <shader/Binding.h>

void Bloom::Initialize(const char *quad_vert_str) {
	{
		m_down_0_shader.Initialize();
		constexpr const char *kFrag =
#include <shader/bloom_down_0.frag.str>
		    ;
		m_down_0_shader.Load(quad_vert_str, GL_VERTEX_SHADER);
		m_down_0_shader.Load(kFrag, GL_FRAGMENT_SHADER);
		m_down_0_shader.Finalize();
	}
	{
		m_down_shader.Initialize();
		constexpr const char *kFrag =
#include <shader/bloom_down.frag.str>
		    ;
		m_down_shader.Load(quad_vert_str, GL_VERTEX_SHADER);
		m_down_shader.Load(kFrag, GL_FRAGMENT_SHADER);
		m_down_shader.Finalize();
	}
	{
		m_up_shader.Initialize();
		constexpr const char *kFrag =
#include <shader/bloom_up.frag.str>
		    ;
		m_up_shader.Load(quad_vert_str, GL_VERTEX_SHADER);
		m_up_shader.Load(kFrag, GL_FRAGMENT_SHADER);
		m_up_shader.Finalize();
	}
}

void Bloom::initialize_target(int width, int height, int mipmap) {
	if (m_width == width && m_height == height && m_mipmap == mipmap)
		return;
	m_width = width;
	m_height = height;
	m_mipmap = mipmap;

	m_bloom.Initialize();
	m_bloom.Storage(width, height, GL_RGB16F, mipmap);
	m_bloom.SetSizeFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	m_bloom.SetWrapFilter(GL_CLAMP_TO_EDGE);

	m_fbos.resize(mipmap);
	GLenum attachments[] = {GL_COLOR_ATTACHMENT0};
	for (int m = 0; m < mipmap; ++m) {
		m_fbos[m].Initialize();
		m_fbos[m].AttachTexture2D(m_bloom, GL_COLOR_ATTACHMENT0, m);
		glNamedFramebufferDrawBuffers(m_fbos[0].Get(), 1, attachments);
	}
	m_bloom.Bind(BLOOM_TEXTURE);
}
