#pragma once

#include "RigidBody.hpp"

struct Sphere final : public RigidBody {
	float radius{}, mass{};

	// inline float GetSDF(const glm::vec3 &p) const { return glm::distance(p, center) - radius; }
	// inline glm::vec3 GetSDFGradient(const glm::vec3 &p) const { return glm::normalize(p - center); }
	inline glm::mat3 GetInertia() const {
		float i = 0.4f * mass * radius * radius;
		return {i, .0f, .0f, .0f, i, .0f, .0f, .0f, i};
	}
};
