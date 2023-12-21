#version 450

#include "Binding.h"

layout(location = 0) out vec3 oColor;

layout(binding = TAA_TEXTURE) uniform sampler2D uTAA;
layout(binding = GBUFFER_VELOCITY_TEXTURE) uniform sampler2D uVelocity;
layout(binding = GBUFFER_DEPTH_TEXTURE) uniform sampler2D uDepth;
layout(binding = MOTION_BLUR_TILE_TEXTURE) uniform sampler2D uTile;

// From Next-Generation-Post-Processing-in-Call-of-Duty-Advanced-Warfare-v18 and Unreal Engine
vec2 DepthCmp(in const float center_depth, in const float sample_depth, in const float depth_scale) {
	return clamp(0.5 + vec2(depth_scale, -depth_scale) * (sample_depth - center_depth), vec2(0), vec2(1));
}
vec2 SpreadCmp(in const float offset_length, in const vec2 spread_length, in const float pixel_to_sample_scale) {
	return clamp(pixel_to_sample_scale * spread_length - max(offset_length - 1., 0.), vec2(0), vec2(1));
}
float SampleWeight(in const float center_depth,
                   in const float sample_depth,
                   in const float offset_length,
                   in const float center_spread_length,
                   in const float sample_spread_length,
                   in const float pixel_to_sample_scale,
                   in const float depth_scale) {
	vec2 depth_weights = DepthCmp(center_depth, sample_depth, depth_scale);
	vec2 spread_weights =
	    SpreadCmp(offset_length, vec2(center_spread_length, sample_spread_length), pixel_to_sample_scale);
	return dot(depth_weights, spread_weights);
}

#define STEP_COUNT 8
#define SOFT_Z_EXTENT 0.5

layout(location = 0) uniform float uInvDeltaT;

float GetVelocityLength(in const vec2 vel_samp) { return length(vel_samp - 0.5) * uInvDeltaT; }

void main() {
	ivec2 coord = ivec2(gl_FragCoord.xy);

	vec2 inv_resolution = 1.0 / textureSize(uTAA, 0);
	vec2 uv = gl_FragCoord.xy * inv_resolution;

	vec3 center_color = texelFetch(uTAA, coord, 0).rgb;
	float center_depth = texelFetch(uDepth, coord, 0).r;
	float center_velocity_length = GetVelocityLength(texelFetch(uVelocity, coord, 0).rg);

	vec2 max_pixel_velocity = texture(uTile, uv).rg - 0.5;
	float max_pixel_velocity_length = length(max_pixel_velocity);

	if (max_pixel_velocity_length < 1e-5) {
		oColor = center_color;
		return;
	}

	vec4 search_vector = vec4(max_pixel_velocity, -max_pixel_velocity);

	float pixel_to_sample_scale = STEP_COUNT / max_pixel_velocity_length;

	vec3 color_accum = vec3(0);
	float weight_accum = 0;

	vec2 jitter = (vec2(coord & 1) - 0.5) * .5;

	for (uint i = 0; i < STEP_COUNT; i++) {
		vec2 offset_length = vec2(i + 0.5) + jitter;
		vec2 offset_fraction = offset_length / STEP_COUNT;

		vec2 sample_uv_0 = uv + offset_fraction * search_vector.xy;
		vec2 sample_uv_1 = uv + offset_fraction * search_vector.zw;

		vec3 sample_color_0 = texture(uTAA, sample_uv_0).rgb;
		vec3 sample_color_1 = texture(uTAA, sample_uv_1).rgb;
		float sample_depth_0 = texture(uDepth, sample_uv_0).r;
		float sample_depth_1 = texture(uDepth, sample_uv_1).r;
		float sample_velocity_length_0 = GetVelocityLength(texture(uVelocity, sample_uv_0).rg);
		float sample_velocity_length_1 = GetVelocityLength(texture(uVelocity, sample_uv_1).rg);

		// in pixels
		float weight_0 = SampleWeight(center_depth, sample_depth_0, offset_length.x, center_velocity_length,
		                              sample_velocity_length_0, pixel_to_sample_scale, SOFT_Z_EXTENT);
		float weight_1 = SampleWeight(center_depth, sample_depth_1, offset_length.x, center_velocity_length,
		                              sample_velocity_length_1, pixel_to_sample_scale, SOFT_Z_EXTENT);

		bvec2 mirror = bvec2(sample_depth_0 > sample_depth_1, sample_velocity_length_1 > sample_velocity_length_0);
		weight_0 = all(mirror) ? weight_1 : weight_0;
		weight_1 = any(mirror) ? weight_1 : weight_0;

		color_accum += weight_0 * sample_color_0 + weight_1 * sample_color_1;
		weight_accum += weight_0 + weight_1;
	}

	color_accum *= .5 / STEP_COUNT;
	weight_accum *= .5 / STEP_COUNT;

	oColor = color_accum + (1 - weight_accum) * center_color;
}
