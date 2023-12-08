#pragma once

#include "Collider.hpp"
#include "GPUMesh.hpp"
#include "Sphere.hpp"
#include "Tumbler.hpp"

#include <algorithm>
#include <array>
#include <optional>
#include <vector>

class Playground {
private:
	std::mt19937 m_rand{std::random_device{}()};

	std::vector<Tumbler> m_tumblers;
	std::vector<Marble> m_marbles;
	std::optional<Fireball> m_fireball;

public:
	struct RayCastInfo {
		float t;
		Tumbler *p_tumbler;
	};
	void Initialize(uint32_t tumbler_count, float place_radius);
	void PopTumblerMesh(GPUMesh *p_mesh) const;
	std::optional<RayCastInfo> RayCastTumbler(const glm::vec3 &origin, const glm::vec3 &dir);

	void PopMarbleMesh(GPUMesh *p_mesh) const;
	void CreateMarbles(uint32_t marble_count, const glm::vec4 &initial_color, float min_speed, float max_speed);
	void DeleteMarbles();

	void PopFireballMesh(GPUMesh *p_mesh) const;
	void CreateFireball(float speed);
	void DeleteFireball();

	template <typename MarbleHitCallback, typename MarbleEmptyCallback, typename FireballHitCallback,
	          typename FireballCallback>
	void Update(float delta_t, MarbleHitCallback &&marble_hit_callback, MarbleEmptyCallback &&marble_empty_callback,
	            FireballHitCallback &&fireball_hit_callback, FireballCallback &&fireball_callback) {
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
			Collider::TestBoundary(&marble, marble_hit_callback);
			for (auto &tumbler : m_tumblers)
				Collider::Test(&marble, &tumbler, marble_hit_callback);
			marble.ApplyGravity(delta_t);
			marble.Update(delta_t);
		}
		if (m_fireball) {
			auto &fireball = m_fireball.value();
			Collider::TestBoundary(&fireball, fireball_hit_callback);
			for (auto &tumbler : m_tumblers)
				Collider::Test(&fireball, &tumbler, fireball_hit_callback);
			for (auto &marble : m_marbles)
				Collider::Test(&fireball, &marble, fireball_hit_callback, marble_hit_callback);
			fireball.Update(delta_t);

			fireball_callback(fireball);
		}
		// remove dead marbles
		m_marbles.erase(std::remove_if(m_marbles.begin(), m_marbles.end(), [](const Marble &m) { return !m.alive; }),
		                m_marbles.end());
		if (m_marbles.empty())
			marble_empty_callback();
	}
};
