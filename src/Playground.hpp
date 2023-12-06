#pragma once

#include "Collider.hpp"
#include "GPUMesh.hpp"
#include "Sphere.hpp"
#include "Tumbler.hpp"

#include <array>
#include <optional>
#include <vector>

class Playground {
private:
	std::mt19937 m_rand{std::random_device{}()};

	std::vector<Tumbler> m_tumblers;
	std::vector<Marble> m_marbles;

public:
	struct RayCastInfo {
		float t;
		Tumbler *p_tumbler;
	};
	void Initialize(uint32_t tumbler_count, float place_radius);
	void PopTumblerMesh(GPUMesh *p_mesh) const;
	std::optional<RayCastInfo> RayCastTumbler(const glm::vec3 &origin, const glm::vec3 &dir);

	void PopMarbleMesh(GPUMesh *p_mesh) const;
	void SplatMarbles(uint32_t marble_count, const glm::vec4 &initial_color);
	void ClearMarbles();

	template <typename MarbleHitCallback> void Update(float delta_t, MarbleHitCallback &&marble_callback) {
		for (uint32_t i = 0; i < m_tumblers.size(); ++i)
			for (uint32_t j = i + 1; j < m_tumblers.size(); ++j)
				Collider::Test(&m_tumblers[i], &m_tumblers[j]);

		for (auto &tumbler : m_tumblers) {
			Collider::TestBoundary(&tumbler);
			tumbler.ApplyFrictionForce(delta_t);
			tumbler.ApplyRecoverForce(delta_t);
		}
		for (auto &tumbler : m_tumblers) {
			if (tumbler.locked) {
				tumbler.angular_velocity = {};
				tumbler.linear_velocity = {};
			}
			tumbler.Update(delta_t);
		}
		for (auto &marble : m_marbles) {
			Collider::TestBoundary(&marble, marble_callback);
			for (auto &tumbler : m_tumblers)
				Collider::Test(&marble, &tumbler, marble_callback);
			marble.ApplyGravity(delta_t);
			marble.Update(delta_t);
		}
	}
};
