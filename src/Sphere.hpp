#pragma once

#include "RigidBody.hpp"

template <typename Derived> struct Sphere : public RigidBody {
	inline static constexpr float kMass = Derived::kMass;
	inline static constexpr float kRadius = Derived::kRadius;

	inline static constexpr glm::mat3 GetInertia() {
		float i = 0.4f * Derived::kMass * Derived::kRadius * Derived::kRadius;
		return {i, .0f, .0f, .0f, i, .0f, .0f, .0f, i};
	}
	inline static constexpr glm::mat3 GetInvInertia() {
		float i = 0.4f * Derived::kMass * Derived::kRadius * Derived::kRadius;
		float inv_i = 1.f / i;
		return {inv_i, .0f, .0f, .0f, inv_i, .0f, .0f, .0f, inv_i};
	}
};

struct Marble : public Sphere<Marble> {
	inline static constexpr float kMass = 1.f;
	inline static constexpr float kRadius = 0.05f;

	glm::vec4 color = {};
};

struct FireBall : public Sphere<Marble> {
	inline static constexpr float kMass = 10.f;
	inline static constexpr float kRadius = 0.1f;
};
