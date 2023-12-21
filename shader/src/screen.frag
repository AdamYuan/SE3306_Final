#version 450

#include "Binding.h"

layout(location = 0) uniform vec2 uJitter;
layout(location = 1) uniform float uInvDeltaT;

layout(location = 0) out vec4 oColor;

layout(binding = BLOOM_TEXTURE) uniform sampler2D uBloom;
layout(binding = MOTION_BLUR_TEXTURE) uniform sampler2D uMotionBlur;
layout(binding = TAA_TEXTURE) uniform sampler2D uTAALight;
layout(binding = MOTION_BLUR_TILE_0_TEXTURE) uniform sampler2D uTile0;

vec3 ToneMapFilmic_Hejl2015(in const vec3 hdr, in const float white_pt) {
	vec4 vh = vec4(hdr, white_pt);
	vec4 va = (1.425 * vh) + 0.05;
	vec4 vf = (vh * va + 0.004) / ((vh * (va + 0.55) + 0.0491)) - 0.0821;
	return vf.rgb / vf.w;
}

float GetVelocityLength(in const vec2 vel_samp) { return length(vel_samp - 0.5) * uInvDeltaT; }

float invlerp(in const float from, in const float to, in const float value) {
	return clamp((value - from) / (to - from), 0.0, 1.0);
}

void main() {
	ivec2 coord = ivec2(gl_FragCoord.xy);
	vec2 inv_resolution = 1.0 / textureSize(uBloom, 0);
	vec2 uv = gl_FragCoord.xy * inv_resolution, uv_unjitter = uv + uJitter * .5;

	vec3 light = texelFetch(uTAALight, coord, 0).rgb;
	vec3 motion = textureLod(uMotionBlur, uv_unjitter, 0).rgb;
	vec3 bloom = textureLod(uBloom, uv_unjitter, 0).rgb;

	float velocity_length = GetVelocityLength(texture(uTile0, uv).rg);
	vec3 color = mix(light, motion, invlerp(0., 1., velocity_length) * .8);
	color /= 1.0 - color; // Inverse Tone Mapping
	color = mix(color, bloom, 0.1);
	color = ToneMapFilmic_Hejl2015(color, 3.2);
	oColor = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);

	// vec2 vtile = texture(uTile, gl_FragCoord.xy / 720).rg;
	// oColor = vec4(abs(vtile - 0.5) * 100, 0, 1);
	// oColor = vec4(vtile, 0, 1);
}
