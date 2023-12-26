#version 450

layout(input_attachment_index = 0, binding = 0) uniform subpassInput uVelocity;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput uDepth;
layout(location = 0) out vec2 oSpeedDepth;

#define SOFT_Z_EXTENT 24.0

void main() {
	vec2 velocity = subpassLoad(uVelocity).rg;
	float depth = subpassLoad(uDepth).r;

	oSpeedDepth = vec2(length(velocity), depth * SOFT_Z_EXTENT);
}
