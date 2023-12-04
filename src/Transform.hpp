#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

struct Velocity {
	glm::vec3 linear, angular;
};

struct Transform {
	glm::vec3 center;
	glm::quat rotate;

	inline glm::mat4 GetRotation() const { return glm::toMat4(rotate); }
	inline glm::mat4 GetModel(float scale) const {
		auto trans = glm::identity<glm::mat4>();
		trans[0][0] = scale;
		trans[1][1] = scale;
		trans[2][2] = scale;
		trans[3] = glm::vec4(center, 1.0);
		return trans * GetRotation();
	}
	inline void Update(const Velocity &v, float delta_t) {
		center += v.linear * delta_t;
		rotate = glm::normalize(rotate + 0.5f * rotate * glm::quat{0.0f, v.angular} * delta_t);
	}
};
