#version 450

#include "Binding.h"
#include "Config.h"
#include "Texture.h"

layout(location = 0) out vec4 oColor;

layout(binding = GBUFFER_ALBEDO_TEXTURE) uniform sampler2D uAlbedo;
layout(binding = BLOOM_TEXTURE) uniform sampler2D uBloom;
layout(binding = LIGHT_TEXTURE) uniform sampler2D uLight;

layout(binding = GBUFFER_PREV_UV_TEXTURE) uniform sampler2D uPrevUV;

void main() {
	ivec2 coord = ivec2(gl_FragCoord.xy);
	vec3 albedo = texelFetch(uAlbedo, coord, 0).rgb;
	bool emissive = IsEmissive(albedo);
	vec3 bloom = texelFetch(uBloom, coord, 0).rgb + (emissive ? albedo : vec3(0));
	vec3 light = texelFetch(uLight, coord, 0).rgb;
	light /= 1. - light; // Inverse tone mapping

	vec3 color = mix(light, bloom, 0.1);
	color = vec3(1) - exp(-color * 1.3);
	oColor = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);

	// oColor = vec4(texelFetch(uPrevUV, coord, 0).xy - gl_FragCoord.xy / 720, 0, 1);
	// oColor.xy *= 1000.0;
}
