#version 450

#include "Binding.h"

layout(location = 0) out vec4 oColor;

layout(binding = GBUFFER_ALBEDO_TEXTURE) uniform sampler2D uAlbedo;
layout(binding = GBUFFER_NORMAL_TEXTURE) uniform sampler2D uNormal;
layout(binding = GBUFFER_DEPTH_TEXTURE) uniform sampler2D uDepth;
layout(binding = SHADOW_MAP_TEXTURE) uniform sampler2DShadow uShadowMap;

layout(std140, binding = CAMERA_UNIFORM_BUFFER) uniform uuCamera {
	mat4 uViewProjection, uInverseViewProjection, uShadowViewProjection;
};

const float kCornellLightHeight = 1.5;

vec3 reconstruct_position(in const vec2 frag_coord, in float depth) {
	vec4 clip = vec4((frag_coord / textureSize(uDepth, 0).xy) * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
	vec4 rec = uInverseViewProjection * clip;
	return rec.xyz / rec.w;
}

vec2 sign_not_zero(in const vec2 v) { return vec2((v.x >= 0.0) ? 1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0); }
vec3 oct_to_float32x3(vec2 e) {
	vec3 v = vec3(e.xy, 1.0 - abs(e.x) - abs(e.y));
	if (v.z < 0)
		v.xy = (1.0 - abs(v.yx)) * sign_not_zero(v.xy);
	return normalize(v);
}

const float kNear = 0.1, kFar = 4.0;
float linearize_depth(in const float depth) { return (2.0 * kNear * kFar) / (kFar + kNear - depth * (kFar - kNear)); }

float compute_visibility(in const vec3 position, in const vec3 normal) {
	float diffuse, shadow;

	{
		vec3 light_dir = normalize(vec3(0, kCornellLightHeight, 0) - position);
		diffuse = max(dot(normal, light_dir), 0.0);
	}
	{
		vec4 shadow_pos = uShadowViewProjection * vec4(position, 1);
		shadow_pos /= shadow_pos.w;
		shadow_pos.xyz = shadow_pos.xyz * 0.5 + 0.5;

		float shadow_size = 1. / textureSize(uShadowMap, 0).x; // square shadow map ensured

		shadow = 0;
#define SHADOW_SAMPLE(OFFSET) shadow += textureProjOffset(uShadowMap, shadow_pos, OFFSET)
#define SHADOW_SAMPLE_X(X_OFFSET) \
	SHADOW_SAMPLE(ivec2(X_OFFSET, -2)); \
	SHADOW_SAMPLE(ivec2(X_OFFSET, -1)); \
	SHADOW_SAMPLE(ivec2(X_OFFSET, 0)); \
	SHADOW_SAMPLE(ivec2(X_OFFSET, 1)); \
	SHADOW_SAMPLE(ivec2(X_OFFSET, 2))
		SHADOW_SAMPLE_X(-2);
		SHADOW_SAMPLE_X(-1);
		SHADOW_SAMPLE_X(0);
		SHADOW_SAMPLE_X(1);
		SHADOW_SAMPLE_X(2);
		shadow *= 0.04;
		shadow = smoothstep(0.02, 1.0, shadow);
	}

	return diffuse * shadow;
}

void main() {
	ivec2 coord = ivec2(gl_FragCoord.xy);
	vec3 albedo = texelFetch(uAlbedo, coord, 0).rgb;
	vec3 normal = oct_to_float32x3(texelFetch(uNormal, coord, 0).rg);
	float depth = texelFetch(uDepth, coord, 0).r;
	vec3 position = reconstruct_position(gl_FragCoord.xy, depth);

	float visibility = compute_visibility(position, normal);

	vec3 emissive = albedo == vec3(1) ? vec3(2) : vec3(0);
	vec3 color = vec3(2) * albedo * visibility + emissive;
	oColor = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);
}
