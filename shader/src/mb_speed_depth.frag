#version 450

#include "Binding.h"

layout(location = 0) out vec2 oSpeedDepth;

layout(binding = GBUFFER_VELOCITY_TEXTURE) uniform sampler2D uVelocity;
layout(binding = GBUFFER_DEPTH_TEXTURE) uniform sampler2D uDepth;

void main() {
	ivec2 coord = ivec2(gl_FragCoord.xy);
	vec2 velocity = texelFetch(uVelocity, coord, 0).rg;
	float depth = texelFetch(uDepth, coord, 0).r;
	oSpeedDepth = vec2(length(velocity), depth);
}
