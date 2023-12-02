#version 450

#include "Binding.h"
#include "Config.h"

layout(location = 0) out vec4 oColor;

layout(binding = GBUFFER_ALBEDO_TEXTURE) uniform sampler2D uAlbedo;
layout(binding = GBUFFER_NORMAL_TEXTURE) uniform sampler2D uNormal;
layout(binding = GBUFFER_DEPTH_TEXTURE) uniform sampler2D uDepth;
layout(binding = SHADOW_MAP_TEXTURE) uniform sampler2DShadow uShadowMap;
layout(binding = VOXEL_RADIANCE_TEXTURE) uniform sampler3D uVoxelRadiance;
layout(binding = VOXEL_RADIANCE_MIPMAP_TEXTURE) uniform sampler3D uVoxelRadianceMipmaps[6];

layout(std140, binding = CAMERA_UNIFORM_BUFFER) uniform uuCamera {
	mat4 uViewProjection, uInverseViewProjection, uShadowViewProjection;
};

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

const float kCornellLightHeight = 1.5;
const vec3 kCornellLightRadiance = vec3(2.0);

float DirectShadow(in const vec3 position, in const vec3 normal) {
	vec4 shadow_pos = uShadowViewProjection * vec4(position, 1);
	shadow_pos /= shadow_pos.w;
	shadow_pos.xyz = shadow_pos.xyz * 0.5 + 0.5;

	float shadow_size = 1. / textureSize(uShadowMap, 0).x; // square shadow map ensured

	float shadow = 0;
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
	return smoothstep(0.02, 1.0, shadow);
}

const vec3 kConeDirections[6] = {vec3(0, 0, 1),
                                 vec3(0, 0.866025, 0.5),
                                 vec3(0.823639, 0.267617, 0.5),
                                 vec3(0.509037, -0.700629, 0.5),
                                 vec3(-0.509037, -0.700629, 0.5),
                                 vec3(-0.823639, 0.267617, 0.5)};
const float kConeWeights[6] = {0.25, 0.15, 0.15, 0.15, 0.15, 0.15};

mat3 normal_to_tbn(in const vec3 normal) {
	vec3 v1 = cross(normal, vec3(0, 0, 1)), v2 = cross(normal, vec3(0, 1, 0));
	vec3 tangent = normalize(dot(v1, v1) > dot(v2, v2) ? v1 : v2);
	return mat3(tangent, cross(tangent, normal), normal);
}

vec4 sample_voxel(in const vec3 position, in const float lod, in const ivec3 axis_indices, in const vec3 axis_weights) {
	vec3 voxel_pos = position * VOXEL_SCALE * .5 + .5;
	if (lod < 1.0)
		return textureLod(uVoxelRadiance, voxel_pos, lod);
	vec4 mipmap_acc = vec4(0);
	float mipmap_lod = lod - 1.0;
	mipmap_acc += axis_weights.x > 0.0
	                  ? axis_weights.x * textureLod(uVoxelRadianceMipmaps[axis_indices.x], voxel_pos, mipmap_lod)
	                  : vec4(0);
	mipmap_acc += axis_weights.y > 0.0
	                  ? axis_weights.y * textureLod(uVoxelRadianceMipmaps[axis_indices.y], voxel_pos, mipmap_lod)
	                  : vec4(0);
	mipmap_acc += axis_weights.z > 0.0
	                  ? axis_weights.z * textureLod(uVoxelRadianceMipmaps[axis_indices.z], voxel_pos, mipmap_lod)
	                  : vec4(0);
	return mipmap_acc;
}

vec3 cone_trace(in const vec3 origin, in const vec3 dir, in const float tan_half_cone) {
	vec4 acc = vec4(0);
	float voxel_size = 2.0 * VOXEL_SCALE / textureSize(uVoxelRadiance, 0).x, inv_voxel_size = 1.0 / voxel_size;
	float dist = 0.2;

	ivec3 axis_indices = ivec3(dir.x < 0.0 ? 0 : 1, dir.y < 0.0 ? 2 : 3, dir.z < 0.0 ? 4 : 5);
	vec3 axis_weights = dir * dir;

	while (dist < 4. && acc.a < 1.) {
		float diameter = 2. * tan_half_cone * dist;
		acc += sample_voxel(origin + dist * dir, log2(diameter * inv_voxel_size), axis_indices, axis_weights) *
		       (1.0 - acc.a);
		dist += diameter * 0.5f;
	}

	return acc.rgb;
}

vec3 IndirectLight(in const vec3 position, in const vec3 normal) {
	mat3 tbn = normal_to_tbn(normal);

	vec3 radiance = vec3(0);
	radiance += kConeWeights[0] * cone_trace(position, normalize(tbn * kConeDirections[0]), 0.57735);
	radiance += kConeWeights[1] * cone_trace(position, normalize(tbn * kConeDirections[1]), 0.57735);
	radiance += kConeWeights[2] * cone_trace(position, normalize(tbn * kConeDirections[2]), 0.57735);
	radiance += kConeWeights[3] * cone_trace(position, normalize(tbn * kConeDirections[3]), 0.57735);
	radiance += kConeWeights[4] * cone_trace(position, normalize(tbn * kConeDirections[4]), 0.57735);
	radiance += kConeWeights[5] * cone_trace(position, normalize(tbn * kConeDirections[5]), 0.57735);

	return radiance;
}

void main() {
	ivec2 coord = ivec2(gl_FragCoord.xy);
	vec3 albedo = texelFetch(uAlbedo, coord, 0).rgb;
	vec3 normal = oct_to_float32x3(texelFetch(uNormal, coord, 0).rg);
	float depth = texelFetch(uDepth, coord, 0).r;
	vec3 position = reconstruct_position(gl_FragCoord.xy, depth);

	vec3 color = albedo == vec3(1)
	                 ? kCornellLightRadiance
	                 : albedo * IndirectLight(position, normal) * (DirectShadow(position, normal) * 0.7 + 0.3);
	oColor = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);
}
