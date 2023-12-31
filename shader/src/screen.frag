#version 450

#include "Binding.h"

layout(location = 0) uniform vec2 uJitter;
layout(location = 1) uniform int uMotionBlurFlag;

layout(location = 0) out vec4 oColor;

layout(binding = BLOOM_TEXTURE) uniform sampler2D uBloom;
layout(binding = MOTION_BLUR_TEXTURE) uniform sampler2D uMotionBlur;
layout(binding = TAA_TEXTURE) uniform sampler2D uTAA;

vec3 ToneMapFilmic_Hejl2015(in const vec3 hdr, in const float white_pt) {
	vec4 vh = vec4(hdr, white_pt);
	vec4 va = (1.425 * vh) + 0.05;
	vec4 vf = (vh * va + 0.004) / ((vh * (va + 0.55) + 0.0491)) - 0.0821;
	return vf.rgb / vf.w;
}

void main() {
	ivec2 coord = ivec2(gl_FragCoord.xy);
	vec2 inv_resolution = 1.0 / textureSize(uBloom, 0);
	vec2 uv = gl_FragCoord.xy * inv_resolution, uv_unjitter = uv + uJitter * .5;

	vec3 color = uMotionBlurFlag == 1 ? texelFetch(uMotionBlur, coord, 0).rgb : texelFetch(uTAA, coord, 0).rgb;
	vec3 bloom = textureLod(uBloom, uv_unjitter, 0).rgb;
	color /= 1.0 - color; // Inverse Tone Mapping
	color = mix(color, bloom, 0.1);
	color = ToneMapFilmic_Hejl2015(color, 3.2);
	oColor = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);
}
