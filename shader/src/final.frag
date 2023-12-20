#version 450

#include "Binding.h"

layout(location = 0) out vec4 oColor;

layout(binding = BLOOM_TEXTURE) uniform sampler2D uBloom;
layout(binding = TAA_TEXTURE) uniform sampler2D uTAALight;

void main() {
	ivec2 coord = ivec2(gl_FragCoord.xy);
	vec3 bloom = texelFetch(uBloom, coord, 0).rgb;
	vec3 light = texelFetch(uTAALight, coord, 0).rgb;
	light /= 1. - light; // Inverse tone mapping

	vec3 color = mix(light, bloom, 0.1);
	color = vec3(1) - exp(-color * 1.3);
	oColor = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);
}
