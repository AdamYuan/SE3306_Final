#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct RigidBody {
protected:
	glm::mat3 rotate_mat{glm::identity<glm::mat3>()}, inv_rotate_mat{glm::identity<glm::mat3>()};

public:
	glm::vec3 center{};
	glm::quat rotate{};
	glm::vec3 linear_velocity{}, angular_velocity{};

	inline void Move(const glm::vec3 &offset) { center += offset; }
	inline void Rotate(const glm::vec3 &angle) {
		rotate = glm::normalize(rotate + 0.5f * rotate * glm::quat{0.0f, angle});
		rotate_mat = glm::toMat3(rotate);
		inv_rotate_mat = glm::transpose(rotate_mat);
	}
	/* inline void RotateByAxis(const glm::vec3 &axis, float angle) {
	    glm::quat quat = {glm::cos(angle * .5f), axis * glm::sin(angle * .5f)};
	    rotate = glm::normalize(rotate * quat);
	    rotate_mat = glm::toMat3(rotate);
	    inv_rotate_mat = glm::transpose(rotate_mat);
	} */

	inline glm::mat4 GetModel() const {
		auto trans = glm::identity<glm::mat4>();
		trans[3] = glm::vec4(center, 1.0);
		return trans * glm::mat4(rotate_mat);
	}
	inline glm::vec3 GetWorldPos(const glm::vec3 &local_pos) const { return rotate_mat * local_pos + center; }
	inline glm::vec3 GetLocalPos(const glm::vec3 &world_pos) const { return inv_rotate_mat * (world_pos - center); }
	inline void Update(float delta_t) {
		Move(linear_velocity * delta_t);
		Rotate(angular_velocity * delta_t);
	}
};
