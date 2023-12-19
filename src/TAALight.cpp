#include "TAALight.hpp"

void TAALight::Initialize(const char *quad_vert_str) {
	m_shader.Initialize();
	constexpr const char *kFrag =
#include <shader/taa_light.frag.str>
	    ;
	m_shader.Load(quad_vert_str, GL_VERTEX_SHADER);
	m_shader.Load(kFrag, GL_FRAGMENT_SHADER);
	m_shader.Finalize();
}

void TAALight::initialize_target(int width, int height) {
	if (m_width == width && m_height == height)
		return;
	m_width = width;
	m_height = height;
	m_tick = 0;

	GLenum attachments[] = {GL_COLOR_ATTACHMENT0};
	for (int i = 0; i < 2; ++i) {
		m_lights[i].Initialize();
		m_lights[i].Storage(m_width, m_height, GL_RGB16, 1);
		m_lights[i].SetSizeFilter(GL_LINEAR, GL_LINEAR);
		m_lights[i].SetWrapFilter(GL_CLAMP_TO_BORDER);

		m_fbos[i].Initialize();
		m_fbos[i].AttachTexture2D(m_lights[i], GL_COLOR_ATTACHMENT0);
		glNamedFramebufferDrawBuffers(m_fbos[i].Get(), 1, attachments);
	}
}

glm::vec2 TAALight::GetJitter(int width, int height) {
	initialize_target(width, height);
	return {};
}
