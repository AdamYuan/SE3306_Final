#include "SSAOBuffer.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <random>
#include <vector>

inline static constexpr uint32_t kSampleCount = 64, kNoiseWidth = 4, kNoiseHeight = 4;

void SSAOBuffer::initialize(int width, int height) {
	// init sample and noise
	if (m_width == -1) {
		std::uniform_real_distribution<float> distr(0.0, 1.0); // random floats between [0.0, 1.0]
		std::mt19937 gen{123};
		std::vector<glm::vec3> samples;
		samples.reserve(kSampleCount);
		for (uint32_t i = 0; i < kSampleCount; ++i) {
			glm::vec3 sample(distr(gen) * 2.f - 1.f, distr(gen) * 2.f - 1.f, distr(gen));
			sample = glm::normalize(sample);
			sample *= distr(gen);
			samples.push_back(sample);
		}

		m_sample.Initialize();
		m_sample.Storage(kSampleCount, GL_RGB16F, 1);
		m_sample.Data((GLvoid *)samples.data(), kSampleCount, GL_RGB, GL_FLOAT, 0);

		std::vector<glm::vec2> noise;
		noise.reserve(kNoiseWidth * kNoiseHeight);
		for (uint32_t i = 0; i < kNoiseWidth * kNoiseHeight; i++)
			noise.emplace_back(distr(gen) * 2.f - 1.f, distr(gen) * 2.f - 1.f);

		m_noise.Initialize();
		m_noise.Storage(kNoiseWidth, kNoiseHeight, GL_RG16F, 1);
		m_noise.Data((GLvoid *)noise.data(), kNoiseWidth, kNoiseHeight, GL_RG, GL_FLOAT, 0);
	}
	if (m_width == width && m_height == height)
		return;
	m_width = width;
	m_height = height;

	m_occlusion.Initialize();
	m_occlusion.Storage(m_width, m_height, GL_R16, 1);
	m_occlusion.SetSizeFilter(GL_LINEAR, GL_LINEAR);
	m_occlusion.SetWrapFilter(GL_CLAMP_TO_EDGE);

	m_fbo.Initialize();
	m_fbo.AttachTexture2D(m_occlusion, GL_COLOR_ATTACHMENT0);

	GLenum attachments[] = {GL_COLOR_ATTACHMENT0};
	glNamedFramebufferDrawBuffers(m_fbo.Get(), 1, attachments);
}
