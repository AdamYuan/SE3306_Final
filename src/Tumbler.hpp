#pragma once

#include <gcem.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/glm.hpp>

#include <random>

#include "Transform.hpp"

class Tumbler final : public Transform {
public:
	// parameters
	inline static constexpr float kBottomRadius = 1.0f, kTopRadius = 0.5f, kHalfAngle = glm::pi<float>() / 9.0f,
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
	inline static constexpr glm::mat3 get_inertia() {
		// integrals
		constexpr float kA = kHalfAngle;
		constexpr float kSinA = gcem::sin(kHalfAngle);
		constexpr float kCosA = gcem::cos(kHalfAngle);
		constexpr float kTanA = gcem::tan(kHalfAngle);
		constexpr float kB = kBottomRadius;
		constexpr float kB2 = gcem::pow(kBottomRadius, 2);
		constexpr float kB3 = gcem::pow(kBottomRadius, 3);
		// constexpr float kB4 = gcem::pow(kBottomRadius, 4);
		constexpr float kB5 = gcem::pow(kBottomRadius, 5);
		constexpr float kT = kTopRadius;
		constexpr float kT2 = gcem::pow(kTopRadius, 2);
		constexpr float kT3 = gcem::pow(kTopRadius, 3);
		constexpr float kT4 = gcem::pow(kTopRadius, 4);
		constexpr float kT5 = gcem::pow(kTopRadius, 5);
		constexpr float kBottom2 = kB3 * (2.0f - kSinA) * gcem::pow(kSinA + 1.0f, 2) / 3.0f;
		constexpr float kY2Bottom2 = 1.0f / 15.0f * kB5 * (-3.f * gcem::pow(kSinA, 5) + 5.f * gcem::pow(kSinA, 3) + 2);
		constexpr float kBottom4 =
		    kB5 * (3.0f * gcem::pow(kSinA, 2) - 9.0f * kSinA + 8.0f) * gcem::pow(kSinA + 1.0f, 3) / 15.0f;
		constexpr float kTop2 = kT3 * (gcem::pow(kSinA, 3) - 3.f * kSinA + 2.0f) / 3.0f;
		constexpr float kH = kTopSphereY;
		constexpr float kH2 = kH * kH;
		constexpr float kY2Top2 =
		    1.0f / 30.0f * kT3 * gcem::pow(kSinA - 1.f, 2) *
		    (2.f * kSinA * (5.f * kH2 + 15.f * kH * kT + 4.f * kT2) + 3.f * kT * kSinA * kSinA * (5.f * kH + 4.f * kT) +
		     6.f * kT2 * gcem::pow(kSinA, 3) + 20.f * kH2 + 15.f * kH * kT + 4.f * kT2);
		constexpr float kTop4 =
		    kT5 * gcem::pow(1.0f - kSinA, 3) * (3.0f * gcem::pow(kSinA, 2) + 9.0f * kSinA + 8.0f) / 15.0f;
		constexpr float kMiddle2 = gcem::pow(kCosA, 3) * (kB3 - kT3) / kTanA / 3.0f;
		constexpr float kY2Middle2 = -1.0f / 120.0f * kCosA / gcem::pow(kTanA, 3) *
		                             (-3.0f * gcem::cos(4.f * kA) * (kB5 - kT5) +
		                              6.f * gcem::cos(2.f * kA) * (3.f * kB5 - 5.f * kB * kT4 + 2.f * kT5) -
		                              19.f * kB5 + 40.f * kB2 * kT3 - 30.f * kB * kT4 + 9.f * kT5);
		constexpr float kMiddle4 = gcem::pow(kCosA, 5) * (kB5 - kT5) / kTanA / 5.0f;
		constexpr float kInt2 = kBottom2 + kMiddle2 + kTop2;
		constexpr float kY2Int2 = kY2Bottom2 + kY2Middle2 + kY2Top2;
		constexpr float kInt4 = kBottom4 + kMiddle4 + kTop4;

		constexpr float kPiRou = kMass / kInt2;
		constexpr float kI0 = 0.5f * kPiRou * kInt4;
		constexpr float kI1 = 0.25f * kPiRou * (kInt4 + 4 * kY2Int2);
		constexpr glm::mat3 kInertia = glm::mat3{kI1, 0.0f, 0.0f, 0.0f, kI0, 0.0f, 0.0f, 0.0f, kI1};
		return kInertia;
	}
	friend class MeshLoader;

public:
	/* inline static glm::mat3 get_inertia_sample(uint32_t samples) {
	    glm::vec3 aabb_min = {-kBottomRadius, -kBottomRadius, -kBottomRadius};
	    glm::vec3 aabb_max = {kBottomRadius, kTopSphereY + kTopRadius, kBottomRadius};
	    std::mt19937 gen{std::random_device{}()};
	    std::uniform_real_distribution<float> x_dis{aabb_min.x, aabb_max.x}, y_dis{aabb_min.y, aabb_max.y},
	        z_dis{aabb_min.z, aabb_max.z};

	    glm::mat3 inertia{};

	    uint32_t actual_samples = 0;
	    for (uint32_t s = 0; s < samples; ++s) {
	        glm::vec3 p = {x_dis(gen), y_dis(gen), z_dis(gen)};
	        if (get_sdf(p) <= 0.0f) {
	            ++actual_samples;
	            inertia[0][0] += p.y * p.y + p.z * p.z;
	            inertia[0][1] -= p.x * p.y;
	            inertia[0][2] -= p.x * p.z;

	            inertia[1][0] -= p.y * p.x;
	            inertia[1][1] += p.x * p.x + p.z * p.z;
	            inertia[1][2] -= p.y * p.z;

	            inertia[2][0] -= p.z * p.x;
	            inertia[2][1] -= p.z * p.y;
	            inertia[2][2] += p.x * p.x + p.y * p.y;
	        }
	    }

	    printf("actual: %u\n", actual_samples);
	    float box_volume = (aabb_max.x - aabb_min.x) * (aabb_max.y - aabb_min.y) * (aabb_max.z - aabb_min.z);
	    printf("volume: %f\n", (float)actual_samples / (float)samples * box_volume);

	    float rou = kMass / float(actual_samples);
	    for (int i = 0; i < 3; ++i)
	        for (int j = 0; j < 3; ++j)
	            inertia[i][j] *= rou;

	    return inertia;
	} */
};
