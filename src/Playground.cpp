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

std::optional<Playground::LockInfo> Playground::TryLockTumbler(const glm::vec3 &origin, const glm::vec3 &dir) {
	m_opt_lock = std::nullopt;
	float min_t = FLT_MAX;
	bool below_center = false;
	for (uint32_t i = 0; i < m_tumblers.size(); ++i) {
		auto opt_t = m_tumblers[i].RayCast(origin, dir);
		if (opt_t.has_value()) {
			float t = opt_t.value();
			if (t < min_t) {
				min_t = t;
				below_center = m_tumblers[i].GetLocalPos(origin + t * dir).y < 0.0f;
				m_opt_lock = i;
			}
		}
	}
	return m_opt_lock.has_value() ? std::optional<LockInfo>({.t = min_t, .below_center = below_center}) : std::nullopt;
}

void Playground::MoveLockedTumbler(const glm::vec2 &offset) {
	if (!m_opt_lock)
		return;
	if (offset == glm::vec2{})
		return;
	m_tumblers[m_opt_lock.value()].center += glm::vec3{offset.x, 0.0f, offset.y};
}

void Playground::RotateLockedTumbler(const glm::vec2 &offset, float rotate_angle) {
	if (!m_opt_lock)
		return;
	if (offset == glm::vec2{})
		return;
	m_tumblers[m_opt_lock.value()].RotateGround(glm::normalize(offset) * rotate_angle);
}

void Playground::SetTumblerMesh(GPUMesh *p_mesh, uint32_t begin_id) const {
	for (uint32_t i = 0; i < m_tumblers.size(); ++i)
		p_mesh->SetModel(begin_id + i, m_tumblers[i].GetModel());
}
void Playground::UnlockTumbler() {
	if (m_opt_lock.has_value()) {
		auto idx = m_opt_lock.value();
		m_tumblers[idx].angular_velocity = {};
		m_tumblers[idx].linear_velocity = {};
	}
	m_opt_lock = std::nullopt;
}
void Playground::Update(float delta_t) {
	for (uint32_t i = 0; i < m_tumblers.size(); ++i)
		for (uint32_t j = i + 1; j < m_tumblers.size(); ++j)
			Collider::Test(&m_tumblers[i], &m_tumblers[j]);

	for (auto &tumbler : m_tumblers) {
		Collider::TestBoundary(&tumbler);
		tumbler.ApplyFrictionForce(delta_t);
		tumbler.ApplyRecoverForce(delta_t);
	}
	if (m_opt_lock.has_value()) {
		auto idx = m_opt_lock.value();
		m_tumblers[idx].angular_velocity = {};
		m_tumblers[idx].linear_velocity = {};
	}
	for (auto &tumbler : m_tumblers)
		tumbler.Update(delta_t);
}
