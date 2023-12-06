#include "Playground.hpp"

#include <cfloat>
#include <random>

void Playground::Initialize(uint32_t tumbler_count, float place_radius) {
	float unit_angle = 2.0f * glm::pi<float>() / float(tumbler_count);
	float first_angle = std::uniform_real_distribution<float>(0.0f, unit_angle)(m_rand);

	m_tumblers.resize(tumbler_count);
	for (uint32_t i = 0; i < m_tumblers.size(); ++i) {
		float angle = first_angle + float(i) * unit_angle;
		m_tumblers[i].center = {place_radius * glm::cos(angle), -1.0f + Tumbler::kBottomRadius,
		                        place_radius * glm::sin(angle)};
	}
}

std::optional<Playground::RayCastInfo> Playground::RayCastTumbler(const glm::vec3 &origin, const glm::vec3 &dir) {
	Tumbler *p_tumbler = nullptr;
	float min_t = FLT_MAX;
	// bool below_center = false;
	for (uint32_t i = 0; i < m_tumblers.size(); ++i) {
		auto opt_t = m_tumblers[i].RayCast(origin, dir);
		if (opt_t.has_value()) {
			float t = opt_t.value();
			if (t < min_t) {
				min_t = t;
				p_tumbler = m_tumblers.data() + i;
				// below_center = m_tumblers[i].GetLocalPos(origin + t * dir).y < 0.0f;
			}
		}
	}
	return p_tumbler ? std::optional<RayCastInfo>({.t = min_t, .p_tumbler = p_tumbler}) : std::nullopt;
}

void Playground::PopTumblerMesh(GPUMesh *p_mesh) const {
	for (uint32_t i = 0; i < m_tumblers.size(); ++i)
		p_mesh->SetModel(i, m_tumblers[i].GetModel());
}
void Playground::PopMarbleMesh(GPUMesh *p_mesh) const {
	uint32_t cnt = 0;
	for (const auto &marble : m_marbles) {
		if (!marble.alive)
			continue;
		p_mesh->SetColor(cnt, marble.color);
		p_mesh->SetModel(cnt, marble.GetModel());
		++cnt;
	}
	p_mesh->SetMeshCount(cnt);
}

void Playground::SplatMarbles(uint32_t marble_count, const glm::vec4 &initial_color) {
	m_marbles.resize(marble_count, {.color = initial_color, .alive = true});

	std::uniform_real_distribution<float> dir_dis{-1.f, 1.f}, speed_dis{2.f, 4.f};
	for (auto &marble : m_marbles) {
		glm::vec3 dir;
		do {
			dir = {dir_dis(m_rand), dir_dis(m_rand), dir_dis(m_rand)};
		} while (dir == glm::vec3{});
		marble.linear_velocity = glm::normalize(dir) * speed_dis(m_rand);
	}
}

void Playground::ClearMarbles() { m_marbles.clear(); }
