#version 450
//
// This shader performs downsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.
// This particular method was customly designed to eliminate
// "pulsating artifacts and temporal stability issues".

// Remember to add bilinear minification filter for this texture!
// Remember to use a floating-point texture format (for HDR)!
// Remember to use edge clamping for this texture!

#include "Binding.h"

layout(binding = BLOOM_TEXTURE) uniform sampler2D uBloom;
layout(location = 0) out vec3 oDownSample;
layout(location = 0) uniform int uSourceLod;

void main() {
	ivec2 src_resolution = textureSize(uBloom, uSourceLod), dst_resolution = textureSize(uBloom, uSourceLod + 1);
	vec2 texcoord = gl_FragCoord.xy / vec2(dst_resolution);
	vec2 src_texel_size = 1.0 / vec2(src_resolution);
	float x = src_texel_size.x;
	float y = src_texel_size.y;

	// Take 13 samples around current texel:
	// a - b - c
	// - j - k -
	// d - e - f
	// - l - m -
	// g - h - i
	// === ('e' is the current texel) ===
	vec3 a = textureLod(uBloom, vec2(texcoord.x - 2 * x, texcoord.y + 2 * y), uSourceLod).rgb;
	vec3 b = textureLod(uBloom, vec2(texcoord.x, texcoord.y + 2 * y), uSourceLod).rgb;
	vec3 c = textureLod(uBloom, vec2(texcoord.x + 2 * x, texcoord.y + 2 * y), uSourceLod).rgb;

	vec3 d = textureLod(uBloom, vec2(texcoord.x - 2 * x, texcoord.y), uSourceLod).rgb;
	vec3 e = textureLod(uBloom, vec2(texcoord.x, texcoord.y), uSourceLod).rgb;
	vec3 f = textureLod(uBloom, vec2(texcoord.x + 2 * x, texcoord.y), uSourceLod).rgb;

	vec3 g = textureLod(uBloom, vec2(texcoord.x - 2 * x, texcoord.y - 2 * y), uSourceLod).rgb;
	vec3 h = textureLod(uBloom, vec2(texcoord.x, texcoord.y - 2 * y), uSourceLod).rgb;
	vec3 i = textureLod(uBloom, vec2(texcoord.x + 2 * x, texcoord.y - 2 * y), uSourceLod).rgb;

	vec3 j = textureLod(uBloom, vec2(texcoord.x - x, texcoord.y + y), uSourceLod).rgb;
	vec3 k = textureLod(uBloom, vec2(texcoord.x + x, texcoord.y + y), uSourceLod).rgb;
	vec3 l = textureLod(uBloom, vec2(texcoord.x - x, texcoord.y - y), uSourceLod).rgb;
	vec3 m = textureLod(uBloom, vec2(texcoord.x + x, texcoord.y - y), uSourceLod).rgb;

	// Apply weighted distribution:
	// 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
	// a,b,d,e * 0.125
	// b,c,e,f * 0.125
	// d,e,g,h * 0.125
	// e,f,h,i * 0.125
	// j,k,l,m * 0.5
	// This shows 5 square areas that are being sampled. But some of them overlap,
	// so to have an energy preserving downsample we need to make some adjustments.
	// The weights are the distributed, so that the sum of j,k,l,m (e.g.)
	// contribute 0.5 to the final color output. The code below is written
	// to effectively yield this sum. We get:
	// 0.125*5 + 0.03125*4 + 0.0625*4 = 1
	oDownSample = e * 0.125;
	oDownSample += (a + c + g + i) * 0.03125;
	oDownSample += (b + d + f + h) * 0.0625;
	oDownSample += (j + k + l + m) * 0.125;
}
