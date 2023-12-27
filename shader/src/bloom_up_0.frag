#version 450

// https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom
//
// This shader performs upsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.

// Remember to add bilinear minification filter for this texture!
// Remember to use a floating-point texture format (for HDR)!
// Remember to use edge clamping for this texture!

layout(constant_id = 0) const float kFilterRadius = 0.0;

layout(binding = 0) uniform sampler2D uBloom;
layout(input_attachment_index = 0, binding = 1) uniform subpassInput uAlbedo;

layout(location = 0) out vec4 oUpSample;

layout(push_constant) uniform uuPushConstant { ivec2 uDstResolution; };

#include "Config.h"
vec3 sample_emissive() {
	vec3 color = subpassLoad(uAlbedo).rgb;
	return IsEmissive(color) ? color : vec3(0);
}

void main() {
	vec2 uv = gl_FragCoord.xy / vec2(uDstResolution);

	// The filter kernel is applied with a radius, specified in texture
	// coordinates, so that the radius will vary across mip resolutions.
	float x = kFilterRadius, y = kFilterRadius;

	// Take 9 samples around current texel:
	// a - b - c
	// d - e - f
	// g - h - i
	// === ('e' is the current texel) ===
	vec3 a = texture(uBloom, vec2(uv.x - x, uv.y + y)).rgb;
	vec3 b = texture(uBloom, vec2(uv.x, uv.y + y)).rgb;
	vec3 c = texture(uBloom, vec2(uv.x + x, uv.y + y)).rgb;

	vec3 d = texture(uBloom, vec2(uv.x - x, uv.y)).rgb;
	vec3 e = texture(uBloom, vec2(uv.x, uv.y)).rgb;
	vec3 f = texture(uBloom, vec2(uv.x + x, uv.y)).rgb;

	vec3 g = texture(uBloom, vec2(uv.x - x, uv.y - y)).rgb;
	vec3 h = texture(uBloom, vec2(uv.x, uv.y - y)).rgb;
	vec3 i = texture(uBloom, vec2(uv.x + x, uv.y - y)).rgb;

	// Apply weighted distribution, by using a 3x3 tent filter:
	//  1   | 1 2 1 |
	// -- * | 2 4 2 |
	// 16   | 1 2 1 |
	vec3 up = e * 4.0;
	up += (b + d + f + h) * 2.0;
	up += (a + c + g + i);
	up *= 1.0 / 16.0;
	up += sample_emissive();
	oUpSample = vec4(up, 1.0);
}
