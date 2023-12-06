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

enum class SphereHitType { kLeft, kRight, kBottom, kTop, kBack, kFront, kLight, kTumbler, kSphere };

struct Marble final : public Sphere<Marble> {
	inline static constexpr float kMass = .15f;
	inline static constexpr float kRadius = 0.06f;

	glm::vec4 color = {};
	bool alive = true;

	inline void ApplyGravity(float delta_t) { linear_velocity.y -= kGravity * delta_t; }
};

struct Fireball final : public Sphere<Marble> {
	inline static constexpr float kMass = 1.f;
	inline static constexpr float kRadius = 0.1f;
};
