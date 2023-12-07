#ifndef SHADER_CONFIG_H
#define SHADER_CONFIG_H

#define VOXEL_SCALE 0.75f
#define Z_NEAR 0.1f
#define Z_FAR 4.0f

#ifndef GLSL
#include <glm/glm.hpp>
#define vec3 glm::vec3
#define const constexpr
#endif

const float kCornellLightHeight = 1.5f, kCornellLightRadius = 0.6f;
const vec3 kCornellLightRadiance = vec3(3.5);

#undef vec3
#undef const

#endif
