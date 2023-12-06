#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct RigidBody {
public:
	inline static constexpr float kGravity = .5f;
	inline static constexpr float kGroundMu = 0.1f;

protected:
	glm::mat3 rotate_mat{glm::identity<glm::mat3>()}, inv_rotate_mat{glm::identity<glm::mat3>()};

public:
	glm::vec3 center{};
	glm::quat rotate{};
	glm::vec3 linear_velocity{}, angular_velocity{};

	inline void Move(const glm::vec3 &offset) { center += offset; }
	inline void Rotate(const glm::vec3 &angle) {
		// angle should be converted to local space
		rotate = glm::normalize(rotate + .5f * rotate * glm::quat{0.0f, inv_rotate_mat * angle});
		rotate_mat = glm::toMat3(rotate);
		inv_rotate_mat = glm::transpose(rotate_mat);
	}
	/* inline void Rotate(const glm::vec3 &axis) {
	    float angle = glm::length(axis);
	    if (angle == .0f)
	        return;
	    glm::quat quat = {glm::cos(angle * .5f), glm::normalize(axis) * glm::sin(angle * .5f)};
	    rotate = glm::normalize(rotate * quat);
	    rotate_mat = glm::toMat3(rotate);
	    inv_rotate_mat = glm::transpose(rotate_mat);
	} */

	inline glm::mat4 GetModel() const {
		auto model = glm::mat4(rotate_mat);
		model[3] = glm::vec4(center, 1.0);
		return model;
	}
	inline glm::vec3 GetWorldPos(const glm::vec3 &local_pos) const { return rotate_mat * local_pos + center; }
	inline glm::vec3 GetLocalPos(const glm::vec3 &world_pos) const { return inv_rotate_mat * (world_pos - center); }
	inline void Update(float delta_t) {
		Move(linear_velocity * delta_t);
		Rotate(angular_velocity * delta_t);
	}
};
