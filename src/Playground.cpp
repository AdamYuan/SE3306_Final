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

template <typename Rand> inline static glm::vec3 gen_random_dir(Rand *p_rand) {
	std::uniform_real_distribution<float> dir_dis{-1.f, 1.f};
	glm::vec3 dir;
	do {
		dir = {dir_dis(*p_rand), dir_dis(*p_rand), dir_dis(*p_rand)};
	} while (dir == glm::vec3{} || glm::length(dir) > 1.f);
	return glm::normalize(dir);
}

void Playground::CreateMarbles(uint32_t marble_count, const glm::vec4 &initial_color, float min_speed,
                               float max_speed) {
	m_marbles.clear();
	m_marbles.resize(marble_count, {.color = initial_color, .alive = true});

	std::uniform_real_distribution<float> dir_dis{-1.f, 1.f}, speed_dis{min_speed, max_speed};
	for (auto &marble : m_marbles)
		marble.linear_velocity = gen_random_dir(&m_rand) * speed_dis(m_rand);
}

void Playground::DeleteMarbles() { m_marbles.clear(); }

void Playground::CreateFireball(float speed) {
	m_fireball = Fireball{};
	auto &fireball = m_fireball.value();
	fireball.linear_velocity = gen_random_dir(&m_rand) * speed;
}
void Playground::DeleteFireball() { m_fireball = std::nullopt; }

void Playground::pop_mesh(GPUMesh *p_tumbler_mesh, GPUMesh *p_marble_mesh, GPUMesh *p_fireball_mesh) const {
	p_tumbler_mesh->SetInstanceCount(m_tumblers.size());
	for (uint32_t i = 0; i < m_tumblers.size(); ++i)
		p_tumbler_mesh->SetModel(i, m_tumblers[i].GetModel());

	p_marble_mesh->SetInstanceCount(m_marbles.size());
	for (uint32_t i = 0; i < m_marbles.size(); ++i) {
		p_marble_mesh->SetColor(i, m_marbles[i].color);
		p_marble_mesh->SetModel(i, m_marbles[i].GetModel());
	}

	if (m_fireball) {
		p_fireball_mesh->SetInstanceCount(1);
		p_fireball_mesh->SetModel(0, m_fireball.value().GetModel());
	} else
		p_fireball_mesh->SetInstanceCount(0);
}
