#ifndef SHADER_CONFIG_H
#define SHADER_CONFIG_H

#define VOXEL_SCALE 0.75f
#define Z_NEAR 0.1f
#define Z_FAR 4.0f
#define VELOCITY_SCALE 16.0f
#define INV_VELOCITY_SCALE 0.0625f

#ifndef GLSL
#include <glm/glm.hpp>
#define vec3 glm::vec3
#define const constexpr
#endif

const float kCornellLightHeight = 1.5f, kCornellLightRadius = 0.6f;
const vec3 kCornellLightRadiance = vec3(3.5);

#ifdef GLSL

bool IsEmissive(in const vec3 color) { return any(greaterThan(color, vec3(1))); }

vec3 GetCornellLightDir(in const vec3 position) { return normalize(vec3(0, kCornellLightHeight, 0) - position); }

float GetCornellLightVisibility(in const vec3 normal, in const vec3 light_dir, in const float shadow) {
	float diffuse = max(dot(normal, light_dir), 0.0);
	float cosine_half_light = (kCornellLightHeight - 1) / kCornellLightRadius;
	float light_edge = smoothstep(0.45, 0.6, dot(light_dir, vec3(0, 1, 0)));
	return min(diffuse, min(shadow, light_edge));
}

#endif

#undef vec3
#undef const

#endif
