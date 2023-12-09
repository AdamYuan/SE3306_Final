#pragma once

#include <bitset>
#include <shader/Config.h>

#include "Sphere.hpp"
#include "Tumbler.hpp"

struct Collider {
	inline static constexpr float kTumblerBoundaryRestitution = .5f;
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
					velocity_update_flags[axis] = true;
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
				p_tumbler->linear_velocity[axis] = -p_tumbler->linear_velocity[axis] * kTumblerBoundaryRestitution;
				p_tumbler->angular_velocity[axis ^ 2] =
				    -p_tumbler->angular_velocity[axis ^ 2] * kTumblerBoundaryRestitution;
			} else {
				p_tumbler->linear_velocity = {};
				p_tumbler->angular_velocity = {};
			}
		};
		if (velocity_update_flags[0])
			update_velocity(0);
		if (velocity_update_flags[1])
			update_velocity(1);
		if (velocity_update_flags[2])
			update_velocity(2);
	}

	template <typename Derived, typename Callback>
	inline static void TestBoundary(Sphere<Derived> *p_sphere, Callback &&callback) {
		std::optional<SphereHitInfo> opt_hit_info;

		glm::vec3 light_pos = glm::vec3{.0f, kCornellLightHeight, .0f};
		glm::vec3 light_diff = p_sphere->center - light_pos;
		float light_dist = glm::length(light_diff);
		if (light_dist < Derived::kRadius + kCornellLightRadius) {
			glm::vec3 light_norm = glm::normalize(light_diff);
			p_sphere->center += light_norm * (Derived::kRadius + kCornellLightRadius - light_dist);
			p_sphere->linear_velocity = glm::reflect(p_sphere->linear_velocity, light_norm);
			opt_hit_info = {.type = SphereHitType::kLight,
			                .position = light_pos + light_norm * kCornellLightRadius,
			                .gradient = light_norm};
		} else {
			const auto test = [&](auto axis) {
				auto a1 = (axis + 1) % 3, a2 = (a1 + 1) % 3;
				glm::vec3 hit_pos = p_sphere->center, hit_grad = {};
				if (p_sphere->center[axis] - Derived::kRadius < -1.f) {
					p_sphere->center[axis] = -1.f + Derived::kRadius;
					p_sphere->linear_velocity[axis] = -p_sphere->linear_velocity[axis] + 1e-4f;
					p_sphere->angular_velocity[a1] = -p_sphere->linear_velocity[a2] / Derived::kRadius;
					p_sphere->angular_velocity[a2] = p_sphere->linear_velocity[a1] / Derived::kRadius;
					hit_pos[axis] = -1.f;
					hit_grad[axis] = 1.f;
					opt_hit_info = {
					    .type = static_cast<SphereHitType>(axis * 2), .position = hit_pos, .gradient = hit_grad};
				} else if (p_sphere->center[axis] + Derived::kRadius > 1.f) {
					p_sphere->center[axis] = 1.f - Derived::kRadius;
					p_sphere->linear_velocity[axis] = -p_sphere->linear_velocity[axis] - 1e-4f;
					p_sphere->angular_velocity[a1] = p_sphere->linear_velocity[a2] / Derived::kRadius;
					p_sphere->angular_velocity[a2] = -p_sphere->linear_velocity[a1] / Derived::kRadius;
					hit_pos[axis] = 1.f;
					hit_grad[axis] = -1.f;
					opt_hit_info = {
					    .type = static_cast<SphereHitType>(axis * 2 + 1), .position = hit_pos, .gradient = hit_grad};
				}
			};

			test(0);
			test(1);
			test(2);
		}

		if (opt_hit_info)
			callback((Derived *)(p_sphere), opt_hit_info.value());
	}

	inline static constexpr float kTumblerRestitution = .8f;
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
		// v = glm::clamp(v, -1.f, 1.f);

		p_tumbler_0->ApplyMomentum(hit_pos, p_tumbler_0->GetInvK(hit_pos) * -hit_dir * v * kTumblerRestitution,
		                           -.2f * Tumbler::kBottomRadius);
		p_tumbler_1->ApplyMomentum(hit_pos, p_tumbler_1->GetInvK(hit_pos) * hit_dir * v * kTumblerRestitution,
		                           -.2f * Tumbler::kBottomRadius);
		// printf("v0=%f, v1=%f\n", v0, v1);
	}

	template <typename Derived, typename Callback>
	inline static void Test(Sphere<Derived> *p_sphere, Tumbler *p_tumbler, Callback &&callback) {
		float sdf = p_tumbler->GetSDF(p_sphere->center);
		if (sdf >= Derived::kRadius)
			return;

		glm::vec3 dir = p_tumbler->GetSDFGradient(p_sphere->center);
		p_sphere->center += dir * (Derived::kRadius - sdf);
		glm::vec3 hit_pos = p_sphere->center - dir * Derived::kRadius;

		glm::vec3 new_l_v = glm::reflect(p_sphere->linear_velocity, dir);
		p_tumbler->ApplyMomentum(p_sphere->center, (p_sphere->linear_velocity - new_l_v) * Derived::kMass);
		p_sphere->linear_velocity = new_l_v;

		callback(static_cast<Derived *>(p_sphere),
		         SphereHitInfo{.type = SphereHitType::kTumbler, .position = hit_pos, .gradient = dir});
	}
	template <typename FireballCallback, typename MarbleCallback>
	inline static void Test(Fireball *p_fireball, Marble *p_marble, FireballCallback &&fireball_callback,
	                        MarbleCallback &&marble_callback) {
		glm::vec3 dist = p_fireball->center - p_marble->center;
		if (glm::dot(dist, dist) >= Fireball::kRadius * Fireball::kRadius)
			return;
		glm::vec3 hit_pos = p_marble->center, hit_grad = glm::normalize(dist);
		marble_callback(p_marble,
		                SphereHitInfo{.type = SphereHitType::kSphere, .position = hit_pos, .gradient = -hit_grad});
		fireball_callback(p_fireball,
		                  SphereHitInfo{.type = SphereHitType::kSphere, .position = hit_pos, .gradient = hit_grad});
	}
};
