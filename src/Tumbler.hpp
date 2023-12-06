#pragma once

#include <gcem.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <optional>
#include <random>

#include "RigidBody.hpp"

struct Tumbler final : public RigidBody {
public:
	// parameters
	inline static constexpr float kBottomRadius = 0.2f, kTopRadius = 0.1f, kHalfAngle = glm::pi<float>() / 9.0f,
	                              kMass = 5.0f;
	static_assert(kTopRadius <= kBottomRadius);

private:
	inline static constexpr float kTopSphereY = (Tumbler::kBottomRadius - Tumbler::kTopRadius) / gcem::sin(kHalfAngle);
	inline static constexpr float kTopEdgeY = (Tumbler::kBottomRadius - Tumbler::kTopRadius) / gcem::tan(kHalfAngle);
	inline static constexpr glm::mat2 kEdgeMat = {gcem::cos(kHalfAngle), -gcem::sin(kHalfAngle), gcem::sin(kHalfAngle),
	                                              gcem::cos(kHalfAngle)};

	inline static float get_sdf(const glm::vec3 &p) {
		glm::vec2 ry = {glm::sqrt(p.x * p.x + p.z * p.z), p.y};
		glm::vec2 edge = kEdgeMat * ry;
		return edge.y < 0.0f ? glm::length(ry) - kBottomRadius
		                     : (edge.y > kTopEdgeY ? glm::distance(ry, {0.0f, kTopSphereY}) - kTopRadius
		                                           : edge.x - kBottomRadius);
	}
	inline static glm::vec3 get_sdf_gradient(const glm::vec3 &p) {
		glm::vec2 ry = {glm::sqrt(p.x * p.x + p.z * p.z), p.y};
		glm::vec2 edge = kEdgeMat * ry;
		return edge.y < 0.0f ? glm::normalize(p)
		                     : (edge.y > kTopEdgeY ? glm::normalize(p - glm::vec3{.0f, kTopSphereY, .0f})
		                                           : glm::normalize(glm::vec3{p.x, ry.x * gcem::tan(kHalfAngle), p.z}));
	}
	struct Inertia {
		// integrals
		static constexpr float kA = kHalfAngle;
		static constexpr float kSinA = gcem::sin(kHalfAngle);
		static constexpr float kCosA = gcem::cos(kHalfAngle);
		static constexpr float kTanA = gcem::tan(kHalfAngle);
		static constexpr float kB = kBottomRadius;
		static constexpr float kB2 = gcem::pow(kBottomRadius, 2);
		static constexpr float kB3 = gcem::pow(kBottomRadius, 3);
		// static constexpr float kB4 = gcem::pow(kBottomRadius, 4);
		static constexpr float kB5 = gcem::pow(kBottomRadius, 5);
		static constexpr float kT = kTopRadius;
		static constexpr float kT2 = gcem::pow(kTopRadius, 2);
		static constexpr float kT3 = gcem::pow(kTopRadius, 3);
		static constexpr float kT4 = gcem::pow(kTopRadius, 4);
		static constexpr float kT5 = gcem::pow(kTopRadius, 5);
		static constexpr float kBottom2 = kB3 * (2.0f - kSinA) * gcem::pow(kSinA + 1.0f, 2) / 3.0f;
		static constexpr float kY2Bottom2 =
		    1.0f / 15.0f * kB5 * (-3.f * gcem::pow(kSinA, 5) + 5.f * gcem::pow(kSinA, 3) + 2);
		static constexpr float kBottom4 =
		    kB5 * (3.0f * gcem::pow(kSinA, 2) - 9.0f * kSinA + 8.0f) * gcem::pow(kSinA + 1.0f, 3) / 15.0f;
		static constexpr float kTop2 = kT3 * (gcem::pow(kSinA, 3) - 3.f * kSinA + 2.0f) / 3.0f;
		static constexpr float kH = kTopSphereY;
		static constexpr float kH2 = kH * kH;
		static constexpr float kY2Top2 =
		    1.0f / 30.0f * kT3 * gcem::pow(kSinA - 1.f, 2) *
		    (2.f * kSinA * (5.f * kH2 + 15.f * kH * kT + 4.f * kT2) + 3.f * kT * kSinA * kSinA * (5.f * kH + 4.f * kT) +
		     6.f * kT2 * gcem::pow(kSinA, 3) + 20.f * kH2 + 15.f * kH * kT + 4.f * kT2);
		static constexpr float kTop4 =
		    kT5 * gcem::pow(1.0f - kSinA, 3) * (3.0f * gcem::pow(kSinA, 2) + 9.0f * kSinA + 8.0f) / 15.0f;
		static constexpr float kMiddle2 = gcem::pow(kCosA, 3) * (kB3 - kT3) / kTanA / 3.0f;
		static constexpr float kY2Middle2 = -1.0f / 120.0f * kCosA / gcem::pow(kTanA, 3) *
		                                    (-3.0f * gcem::cos(4.f * kA) * (kB5 - kT5) +
		                                     6.f * gcem::cos(2.f * kA) * (3.f * kB5 - 5.f * kB * kT4 + 2.f * kT5) -
		                                     19.f * kB5 + 40.f * kB2 * kT3 - 30.f * kB * kT4 + 9.f * kT5);
		static constexpr float kMiddle4 = gcem::pow(kCosA, 5) * (kB5 - kT5) / kTanA / 5.0f;
		static constexpr float kInt2 = kBottom2 + kMiddle2 + kTop2;
		static constexpr float kY2Int2 = kY2Bottom2 + kY2Middle2 + kY2Top2;
		static constexpr float kInt4 = kBottom4 + kMiddle4 + kTop4;

		static constexpr float kPiRou = kMass / kInt2;
		static constexpr float kI0 = 0.5f * kPiRou * kInt4;
		static constexpr float kI1 = 0.25f * kPiRou * (kInt4 + 4 * kY2Int2);
		static constexpr glm::mat3 kInertia = glm::mat3{kI1, 0.0f, 0.0f, 0.0f, kI0, 0.0f, 0.0f, 0.0f, kI1};
		static constexpr glm::mat3 kInvInertia =
		    glm::mat3{1.0f / kI1, 0.0f, 0.0f, 0.0f, 1.0f / kI0, 0.0f, 0.0f, 0.0f, 1.0f / kI1};
	};
	inline static constexpr glm::mat3 get_inertia() { return Inertia::kInertia; }
	inline static constexpr glm::mat3 get_inv_inertia() { return Inertia::kInvInertia; }
	friend class MeshLoader;
	friend class Collider;

public:
	bool locked = false;

	inline float GetSDF(const glm::vec3 &p) const { return get_sdf(GetLocalPos(p)); }
	inline glm::vec3 GetSDFGradient(const glm::vec3 &p) const { return rotate_mat * get_sdf_gradient(GetLocalPos(p)); }
	inline glm::mat3 GetInertia() const { return rotate_mat * get_inertia() * inv_rotate_mat; }
	inline glm::mat3 GetInvInertia() const { return rotate_mat * get_inv_inertia() * inv_rotate_mat; }
	/* inline void RotateGround(const glm::vec2 &xz_dir) {
	    glm::vec3 dir = {xz_dir[0], 0.f, xz_dir[1]};
	    glm::vec3 axis = {dir.z, 0.f, -dir.x};
	    Rotate(axis);
	} */

	inline void RotateGround(const glm::vec2 &xz_dir) {
		glm::vec3 dir = {xz_dir[0], 0.f, xz_dir[1]};
		glm::vec3 axis = {dir.z, 0.f, -dir.x};
		Rotate(axis);
		center += dir * Tumbler::kBottomRadius;
	}
	/* inline void RotateGroundAxis(const glm::vec2 &xz_axis) {
	    glm::vec3 axis = {xz_axis[0], 0.f, xz_axis[1]};
	    glm::vec3 dir = {-axis.z, 0.f, axis.x};
	    Rotate(axis);
	    center += dir * Tumbler::kBottomRadius;
	} */
	inline void MoveLocked(const glm::vec2 &offset) {
		if (!locked)
			return;
		center += glm::vec3{offset.x, 0.0f, offset.y};
	}
	inline void RotateLocked(const glm::vec2 &offset, float rotate_angle) {
		if (!locked)
			return;
		if (offset == glm::vec2{})
			return;
		RotateGround(glm::normalize(offset) * rotate_angle);
	}
	inline std::optional<float> RayCast(const glm::vec3 &origin, const glm::vec3 &dir, float threshold = 0.0001f,
	                                    uint32_t max_steps = 32) const {
		glm::vec3 local_orig = GetLocalPos(origin), local_dir = inv_rotate_mat * dir;

		float t = 0.0f;
		while (max_steps--) {
			float sdf = get_sdf(local_orig + t * local_dir);
			if (sdf < threshold)
				return t;
			t += sdf;
		}
		return std::nullopt;
	}

	inline void ApplyMomentum(const glm::vec3 &origin, const glm::vec3 &momentum, float center_y_bias = .0f) {
		glm::vec3 fake_center = center;
		fake_center.y += center_y_bias;
		glm::vec3 l = glm::cross(origin - fake_center, momentum); // angular momentum
		glm::vec3 delta_angular_velocity = get_inv_inertia() * l;
		angular_velocity += delta_angular_velocity;
		linear_velocity +=
		    glm::vec3{-delta_angular_velocity.z * kBottomRadius, .0f, delta_angular_velocity.x * kBottomRadius};
	}

	inline glm::vec3 GetTopSpherePos() const { return center + kTopSphereY * rotate_mat[1]; }

	inline void ApplyRecoverForce(float delta_t) {
		glm::vec3 dir = rotate_mat[1];
		glm::vec3 force = -dir;
		glm::vec3 r = glm::vec3{0.f, 1.f, 0.f};
		glm::vec3 t = glm::cross(r, force); // torque
		glm::vec3 l = t * delta_t;          // angular momentum
		glm::vec3 delta_angular_velocity = get_inv_inertia() * l;
		angular_velocity += delta_angular_velocity;
		linear_velocity +=
		    glm::vec3{-delta_angular_velocity.z * kBottomRadius, .0f, delta_angular_velocity.x * kBottomRadius};
	}

	inline void ApplyFrictionForce(float delta_t) {
		if (linear_velocity != glm::vec3{})
			linear_velocity = glm::normalize(linear_velocity) *
			                  glm::max(glm::length(linear_velocity) - kGravity * kGroundMu * delta_t, 0.0f);
		if (angular_velocity != glm::vec3{})
			angular_velocity =
			    glm::normalize(angular_velocity) *
			    glm::max(glm::length(angular_velocity) - kGravity * kGroundMu * delta_t / kBottomRadius, 0.0f);
	}
};
