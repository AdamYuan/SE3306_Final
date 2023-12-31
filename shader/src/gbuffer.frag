#version 450

#include "Config.h"
#include "Texture.h"

layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec3 vColor;
layout(location = 2) in vec4 vClip;
layout(location = 3) in vec4 vPrevClip;

layout(location = 0) out vec3 oAlbedo;
layout(location = 1) out vec2 oNormal;
layout(location = 2) out vec2 oVelocity;

vec2 sign_not_zero(in const vec2 v) { return vec2((v.x >= 0.0) ? 1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0); }
// Assume normalized input. Output is on [-1, 1] for each component.
vec2 float32x3_to_oct(in const vec3 v) {
	// Project the sphere onto the octahedron, and then onto the xy plane
	vec2 p = v.xy * (1.0 / (abs(v.x) + abs(v.y) + abs(v.z)));
	// Reflect the folds of the lower hemisphere over the diagonals
	return (v.z <= 0.0) ? ((1.0 - abs(p.yx)) * sign_not_zero(p)) : p;
}

void main() {
	oAlbedo = GetAlbedo(vColor);
	// since uModel is a guarenteed orthogonal matrix, there's no need for transpose(inverse(mat3(uModel)))
	oNormal = float32x3_to_oct(normalize(vNormal));
	oVelocity = (vClip.xy / vClip.w - vPrevClip.xy / vPrevClip.w) * .5 * VELOCITY_SCALE;
}
