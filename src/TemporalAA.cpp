#include "TemporalAA.hpp"

void TemporalAA::Initialize(const char *quad_vert_str) {
	m_shader.Initialize();
	constexpr const char *kFrag =
#include <shader/taa.frag.str>
	    ;
	m_shader.Load(quad_vert_str, GL_VERTEX_SHADER);
	m_shader.Load(kFrag, GL_FRAGMENT_SHADER);
	m_shader.Finalize();
}

void TemporalAA::initialize_target(int width, int height) {
	if (m_width == width && m_height == height)
		return;
	m_width = width;
	m_height = height;
	m_tick = 0;

	GLenum attachments[] = {GL_COLOR_ATTACHMENT0};
	for (int i = 0; i < 2; ++i) {
		m_textures[i].Initialize();
		m_textures[i].Storage(m_width, m_height, GL_RGB10, 1);
		m_textures[i].SetSizeFilter(GL_LINEAR, GL_LINEAR);
		m_textures[i].SetWrapFilter(GL_CLAMP_TO_BORDER);

		m_fbos[i].Initialize();
		m_fbos[i].AttachTexture2D(m_textures[i], GL_COLOR_ATTACHMENT0);
		glNamedFramebufferDrawBuffers(m_fbos[i].Get(), 1, attachments);
	}
}

glm::vec2 TemporalAA::GetJitter(int width, int height) {
	constexpr const glm::vec2 kJitters[16] = {
	    {0.500000, 0.333333}, {0.250000, 0.666667}, {0.750000, 0.111111}, {0.125000, 0.444444},
	    {0.625000, 0.777778}, {0.375000, 0.222222}, {0.875000, 0.555556}, {0.062500, 0.888889},
	    {0.562500, 0.037037}, {0.312500, 0.370370}, {0.812500, 0.703704}, {0.187500, 0.148148},
	    {0.687500, 0.481481}, {0.437500, 0.814815}, {0.937500, 0.259259}, {0.031250, 0.592593},
	};
	initialize_target(width, height);
	return (kJitters[m_tick & 0xfu] * 2.f - 1.f) / glm::vec2{width, height};
}
