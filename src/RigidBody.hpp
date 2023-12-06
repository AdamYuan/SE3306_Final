#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct RigidBody {
public:
	inline static constexpr float kGravity = 3.0f;
	inline static constexpr float kGroundMu = 0.01f;

protected:
	glm::mat3 rotate_mat{glm::identity<glm::mat3>()}, inv_rotate_mat{glm::identity<glm::mat3>()};

public:
	glm::vec3 center{};
	glm::quat rotate{};
	glm::vec3 linear_velocity{}, angular_velocity{};

	inline void Move(const glm::vec3 &world_offset) { center += world_offset; }
	inline void Rotate(const glm::vec3 &world_angle) {
		// angle should be converted to local space
		rotate = glm::normalize(rotate + .5f * rotate * glm::quat{0.0f, inv_rotate_mat * world_angle});
		rotate_mat = glm::toMat3(rotate);
		inv_rotate_mat = glm::transpose(rotate_mat);
	}

	inline glm::mat4 GetModel() const {
		auto model = glm::mat4(rotate_mat);
		model[3] = glm::vec4(center, 1.0);
		return model;
	}
	inline glm::vec3 GetLocalPos(const glm::vec3 &world_pos) const { return inv_rotate_mat * (world_pos - center); }
	inline void Update(float delta_t) {
		Move(linear_velocity * delta_t);
		Rotate(angular_velocity * delta_t);
	}
	inline glm::vec3 GetVelocity(const glm::vec3 &world_pos) const {
		return linear_velocity + glm::cross(angular_velocity, world_pos - center);
	}
};
