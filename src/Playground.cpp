#include "Playground.hpp"

#include "Collider.hpp"

#include <cfloat>
#include <random>

void Playground::Initialize(uint32_t tumbler_count, float place_radius) {
	std::mt19937 gen{std::random_device{}()};
	float unit_angle = 2.0f * glm::pi<float>() / float(tumbler_count);
	float first_angle = std::uniform_real_distribution<float>(0.0f, unit_angle)(gen);

	m_tumblers.resize(tumbler_count);
	for (uint32_t i = 0; i < m_tumblers.size(); ++i) {
		float angle = first_angle + float(i) * unit_angle;
		m_tumblers[i].center = {place_radius * glm::cos(angle), -1.0f + Tumbler::kBottomRadius,
		                        place_radius * glm::sin(angle)};
	}
}

std::optional<float> Playground::TryLockTumbler(const glm::vec3 &origin, const glm::vec3 &dir) {
	m_opt_lock = std::nullopt;
	float min_t = FLT_MAX;
	for (uint32_t i = 0; i < m_tumblers.size(); ++i) {
		auto opt_t = m_tumblers[i].RayCast(origin, dir);
		if (opt_t.has_value()) {
			float t = opt_t.value();
			if (t < min_t) {
				min_t = t;
				bool trans = m_tumblers[i].GetLocalPos(origin + t * dir).y < 0.0f;
				m_opt_lock = LockInfo{.index = i, .translate = trans};
			}
		}
	}
	return m_opt_lock.has_value() ? std::optional<float>(min_t) : std::nullopt;
}

void Playground::MoveLockedTumbler(const glm::vec2 &offset, float rotate_angle) {
	if (!m_opt_lock)
		return;

	if (offset == glm::vec2{})
		return;

	const auto &lock = m_opt_lock.value();
	if (lock.translate) {
		m_tumblers[lock.index].center += glm::vec3{offset.x, 0.0f, offset.y};
	} else {
		m_tumblers[lock.index].RotateGround(glm::normalize(offset) * rotate_angle);
	}
}

void Playground::SetTumblerMesh(GPUMesh *p_mesh, uint32_t begin_id) const {
	for (uint32_t i = 0; i < m_tumblers.size(); ++i)
		p_mesh->SetModel(begin_id + i, m_tumblers[i].GetModel());
}
void Playground::Update(float delta_t) {
	if (m_opt_lock.has_value()) {
		auto idx = m_opt_lock.value().index;
		m_tumblers[idx].angular_velocity = {};
		m_tumblers[idx].linear_velocity = {};
	}
	for (auto &tumbler : m_tumblers) {
		Collider::TestBoundary(&tumbler);

		tumbler.ApplyRecoverForce(delta_t);
		tumbler.Update(delta_t);
	}
}
