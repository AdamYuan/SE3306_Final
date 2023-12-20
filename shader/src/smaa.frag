#version 450

#include "Binding.h"

layout(location = 0) out vec3 oColor;

layout(binding = LIGHT_TEXTURE) uniform sampler2D uLight;

void main() {
	ivec2 coord = ivec2(gl_FragCoord.xy), resolution = textureSize(uLight, 0);
	vec2 inv_resolution = 1.0 / resolution;
	vec2 uv = gl_FragCoord.xy * inv_resolution;

	vec3 light = texture(uLight, uv).rgb;
	oColor = light;
}
