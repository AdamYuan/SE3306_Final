#pragma once

#include <bitset>

#include "Sphere.hpp"
#include "Tumbler.hpp"

struct Collider {
	inline static constexpr float kBoundaryRestitution = .5f;
	inline static void TestBoundary(Tumbler *p_tumbler) {
		std::bitset<3> velocity_update_flags;

		// bottom
		const auto test_bottom = [&](auto axis) {
			if (p_tumbler->center[axis] - Tumbler::kBottomRadius < -1.f) {
				p_tumbler->center[axis] = -1.f + Tumbler::kBottomRadius;
				velocity_update_flags[axis] = true;
			} else if (p_tumbler->center[axis] + Tumbler::kBottomRadius > 1.f) {
				p_tumbler->center[axis] = 1.f - Tumbler::kBottomRadius;
				velocity_update_flags[axis] = true;
			}
		};
		test_bottom(0);
		test_bottom(2);

		// top
		glm::vec3 top_pos = p_tumbler->GetWorldPos({.0f, Tumbler::kTopSphereY, .0f});

		const auto test_top = [&](auto axis) {
			if (axis == 1) {
				if (top_pos[axis] - Tumbler::kTopRadius < -1.f) {
					glm::vec3 dir = p_tumbler->rotate_mat[1];
					float angle = -glm::asin(dir.y) - Tumbler::kHalfAngle;
					p_tumbler->RotateGround(-glm::normalize(dir.xz()) * angle);
				}
			} else {
				if (top_pos[axis] - Tumbler::kTopRadius < -1.f) {
					p_tumbler->center[axis] -= top_pos[axis] - Tumbler::kTopRadius + 1.0f;
					velocity_update_flags[axis] = true;
				} else if (top_pos[axis] + Tumbler::kTopRadius > 1.f) {
					p_tumbler->center[axis] -= top_pos[axis] + Tumbler::kTopRadius - 1.0f;
					velocity_update_flags[axis] = true;
				}
			}
		};
		test_top(0);
		test_top(1);
		test_top(2);

		const auto update_velocity = [&](auto axis) {
			if (axis != 1) {
				p_tumbler->linear_velocity[axis] = -p_tumbler->linear_velocity[axis] * kBoundaryRestitution;
				p_tumbler->angular_velocity[axis ^ 2] = -p_tumbler->angular_velocity[axis ^ 2] * kBoundaryRestitution;
			}
		};
		if (velocity_update_flags[0])
			update_velocity(0);
		if (velocity_update_flags[2])
			update_velocity(2);
	}
	template <typename CallbackFunc> inline static void TestBoundary(Sphere *p_sphere, CallbackFunc &&callback) {}
	inline static void TestBoundary(Sphere *p_sphere) {
		TestBoundary(p_sphere, []() {});
	}
};
