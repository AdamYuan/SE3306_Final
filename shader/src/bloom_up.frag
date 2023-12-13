#version 450

// This shader performs upsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.

// Remember to add bilinear minification filter for this texture!
// Remember to use a floating-point texture format (for HDR)!
// Remember to use edge clamping for this texture!
//
#include "Binding.h"

layout(binding = BLOOM_TEXTURE) uniform sampler2D uBloom;

layout(location = 0) uniform int uSourceLod;
layout(location = 1) uniform float uFilterRadius;

layout(location = 0) out vec3 oUpSample;

void main() {
	ivec2 dst_resolution = textureSize(uBloom, uSourceLod - 1);
	vec2 texcoord = gl_FragCoord.xy / vec2(dst_resolution);

	// The filter kernel is applied with a radius, specified in texture
	// coordinates, so that the radius will vary across mip resolutions.
	float x = uFilterRadius, y = uFilterRadius;

	// Take 9 samples around current texel:
	// a - b - c
	// d - e - f
	// g - h - i
	// === ('e' is the current texel) ===
	vec3 a = textureLod(uBloom, vec2(texcoord.x - x, texcoord.y + y), uSourceLod).rgb;
	vec3 b = textureLod(uBloom, vec2(texcoord.x, texcoord.y + y), uSourceLod).rgb;
	vec3 c = textureLod(uBloom, vec2(texcoord.x + x, texcoord.y + y), uSourceLod).rgb;

	vec3 d = textureLod(uBloom, vec2(texcoord.x - x, texcoord.y), uSourceLod).rgb;
	vec3 e = textureLod(uBloom, vec2(texcoord.x, texcoord.y), uSourceLod).rgb;
	vec3 f = textureLod(uBloom, vec2(texcoord.x + x, texcoord.y), uSourceLod).rgb;

	vec3 g = textureLod(uBloom, vec2(texcoord.x - x, texcoord.y - y), uSourceLod).rgb;
	vec3 h = textureLod(uBloom, vec2(texcoord.x, texcoord.y - y), uSourceLod).rgb;
	vec3 i = textureLod(uBloom, vec2(texcoord.x + x, texcoord.y - y), uSourceLod).rgb;

	// Apply weighted distribution, by using a 3x3 tent filter:
	//  1   | 1 2 1 |
	// -- * | 2 4 2 |
	// 16   | 1 2 1 |
	oUpSample = e * 4.0;
	oUpSample += (b + d + f + h) * 2.0;
	oUpSample += (a + c + g + i);
	oUpSample *= 1.0 / 16.0;
}
