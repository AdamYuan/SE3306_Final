#include "ShadowMap.hpp"

void ShadowMap::initialize(int width, int height) {
	if (m_width == width && m_height == height)
		return;

	m_width = width;
	m_height = height;

	m_depth.Initialize();
	m_depth.Storage(m_width, m_height, GL_DEPTH_COMPONENT32, 1);
	m_depth.SetSizeFilter(GL_LINEAR, GL_LINEAR);
	m_depth.SetWrapFilter(GL_CLAMP_TO_EDGE);
	glTextureParameteri(m_depth.Get(), GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTextureParameteri(m_depth.Get(), GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	m_fbo.Initialize();
	m_fbo.AttachTexture2D(m_depth, GL_DEPTH_ATTACHMENT);
}
