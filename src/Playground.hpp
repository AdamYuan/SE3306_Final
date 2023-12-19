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

	void pop_mesh_prev(GPUMesh *p_tumbler_mesh, GPUMesh *p_marble_mesh, GPUMesh *p_fireball_mesh) const;
	void pop_mesh(GPUMesh *p_tumbler_mesh, GPUMesh *p_marble_mesh, GPUMesh *p_fireball_mesh) const;

public:
	struct RayCastInfo {
		float t;
		Tumbler *p_tumbler;
	};
	void Initialize(uint32_t tumbler_count, float place_radius);
	std::optional<RayCastInfo> RayCastTumbler(const glm::vec3 &origin, const glm::vec3 &dir);

	void CreateMarbles(uint32_t marble_count, const glm::vec4 &initial_color, float min_speed, float max_speed);
	void DeleteMarbles();

	void CreateFireball(float speed);
	void DeleteFireball();

	template <typename TumblerDragFunc, typename MarbleHitCallback, typename MarbleEmptyCallback,
	          typename FireballHitCallback, typename FireballCallback>
	void Update(float delta_t, GPUMesh *p_tumbler_mesh, GPUMesh *p_marble_mesh, GPUMesh *p_fireball_mesh,
	            TumblerDragFunc &&tumbler_drag_func, MarbleHitCallback &&marble_hit_callback,
	            MarbleEmptyCallback &&marble_empty_callback, FireballHitCallback &&fireball_hit_callback,
	            FireballCallback &&fireball_callback) {
		// remove dead marbles
		m_marbles.erase(std::remove_if(m_marbles.begin(), m_marbles.end(), [](const Marble &m) { return !m.alive; }),
		                m_marbles.end());
		if (m_marbles.empty())
			marble_empty_callback();

		pop_mesh_prev(p_tumbler_mesh, p_marble_mesh, p_fireball_mesh);

		tumbler_drag_func();

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

		pop_mesh(p_tumbler_mesh, p_marble_mesh, p_fireball_mesh);
	}
};
