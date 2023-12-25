#pragma once

#include <glm/glm.hpp>
#include <map>

struct Transform {
	glm::mat4 model;
	glm::vec4 color;
};

using TransformSet = std::map<uint64_t, Transform>;
