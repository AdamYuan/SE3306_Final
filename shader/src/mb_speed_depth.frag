#version 450

#include "Binding.h"

layout(location = 0) uniform vec2 uJitter;

layout(location = 0) out vec2 oSpeedDepth;

layout(binding = GBUFFER_VELOCITY_TEXTURE) uniform sampler2D uVelocity;
layout(binding = GBUFFER_DEPTH_TEXTURE) uniform sampler2D uDepth;

#define SOFT_Z_EXTENT 24.0

void main() {
	vec2 inv_resolution = 1.0 / textureSize(uVelocity, 0);
	vec2 uv_unjitter = gl_FragCoord.xy * inv_resolution + uJitter * .5;

	vec2 velocity = texture(uVelocity, uv_unjitter).rg;
	float depth = texture(uDepth, uv_unjitter).r;

	oSpeedDepth = vec2(length(velocity), depth * SOFT_Z_EXTENT);
}
