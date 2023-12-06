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
		glm::vec3 top_pos = p_tumbler->GetTopSpherePos();

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
	inline static constexpr float kTumblerRestitution = .2f;
	inline static void Test(Tumbler *p_tumbler_0, Tumbler *p_tumbler_1) {
		uint32_t hit_count = 0;
		glm::vec3 hit_pos = {};

		const auto test = [&](const Tumbler *l, const Tumbler *r) {
			glm::vec3 bot_pos = l->center;
			float bot_sdf = r->GetSDF(bot_pos);
			if (bot_sdf < Tumbler::kBottomRadius) {
				++hit_count;
				hit_pos += bot_pos - r->GetSDFGradient(bot_pos) * bot_sdf;
			}
			glm::vec3 top_pos = l->GetTopSpherePos();
			float top_sdf = r->GetSDF(top_pos);
			if (top_sdf < Tumbler::kTopRadius) {
				++hit_count;
				hit_pos += top_pos - r->GetSDFGradient(top_pos) * top_sdf;
			}
		};
		test(p_tumbler_0, p_tumbler_1);
		test(p_tumbler_1, p_tumbler_0);

		if (hit_count == 0)
			return;

		hit_pos /= float(hit_count);
		glm::vec3 hit_dir = (p_tumbler_0->GetSDFGradient(hit_pos) - p_tumbler_1->GetSDFGradient(hit_pos)) * .5f;
		hit_dir = glm::normalize(hit_dir);
		float hit_depth = -(p_tumbler_0->GetSDF(hit_pos) + p_tumbler_1->GetSDF(hit_pos));
		hit_depth = glm::max(.0f, hit_depth);

		glm::vec3 xz_hit_dir = glm::normalize(glm::vec3{hit_dir.x, .0f, hit_dir.z});
		p_tumbler_0->center -= xz_hit_dir * hit_depth;
		p_tumbler_1->center += xz_hit_dir * hit_depth;

		glm::vec3 speed_0 = p_tumbler_0->GetVelocity(hit_pos), speed_1 = p_tumbler_1->GetVelocity(hit_pos);
		float v0 = glm::dot(speed_0, hit_dir), v1 = glm::dot(speed_1, hit_dir);

		float v = v0 - v1;
		v = glm::clamp(v, -1.f, 1.f);
		p_tumbler_0->ApplyMomentum(hit_pos, -hit_dir * v * Tumbler::kMass * kTumblerRestitution);
		p_tumbler_1->ApplyMomentum(hit_pos, hit_dir * v * Tumbler::kMass * kTumblerRestitution);
		// printf("v0=%f, v1=%f\n", v0, v1);
	}
};
