#version 450

#include "Binding.h"
#include "Config.h"
#include "Texture.h"

layout(location = 0) out vec3 oLight;

layout(binding = GBUFFER_ALBEDO_TEXTURE) uniform sampler2D uAlbedo;
layout(binding = GBUFFER_NORMAL_TEXTURE) uniform sampler2D uNormal;
layout(binding = GBUFFER_DEPTH_TEXTURE) uniform sampler2D uDepth;
layout(binding = GBUFFER_PREV_UV_TEXTURE) uniform sampler2D uPrevUV;
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

float linearize_depth(in const float depth) {
	return (2.0 * Z_NEAR * Z_FAR) / (Z_FAR + Z_NEAR - depth * (Z_FAR - Z_NEAR));
}
float de_linearize_depth(in const float linear_depth) {
	return (2.0 * Z_NEAR * Z_FAR / linear_depth - Z_FAR - Z_NEAR) / (Z_NEAR - Z_FAR);
}

float DirectVisibility(in const vec3 position, in const vec3 normal) {
	vec3 light_dir = GetCornellLightDir(position);
	vec4 shadow_pos = uShadowViewProjection * vec4(position, 1);
	shadow_pos /= shadow_pos.w;
	shadow_pos.xyz = shadow_pos.xyz * 0.5 + 0.5;

	shadow_pos.z = de_linearize_depth(linearize_depth(shadow_pos.z) + clamp(0.1 * dot(normal, light_dir), -0.02, 0.05));

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
	shadow = smoothstep(0.02, 1.0, shadow);

	return GetCornellLightVisibility(normal, light_dir, shadow) * .2 + .8;
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
	vec4 mipmap_acc = vec4(0);
	float mipmap_lod = max(lod - 1.0, 0.0);
	mipmap_acc += axis_weights.x > 0.0
	                  ? axis_weights.x * textureLod(uVoxelRadianceMipmaps[axis_indices.x], voxel_pos, mipmap_lod)
	                  : vec4(0);
	mipmap_acc += axis_weights.y > 0.0
	                  ? axis_weights.y * textureLod(uVoxelRadianceMipmaps[axis_indices.y], voxel_pos, mipmap_lod)
	                  : vec4(0);
	mipmap_acc += axis_weights.z > 0.0
	                  ? axis_weights.z * textureLod(uVoxelRadianceMipmaps[axis_indices.z], voxel_pos, mipmap_lod)
	                  : vec4(0);
	return lod >= 1.0 ? mipmap_acc : mix(texture(uVoxelRadiance, voxel_pos), mipmap_acc, lod);
}

vec3 cone_trace(in const vec3 origin,
                in const vec3 dir,
                in const float tan_half_cone,
                in const float voxel_size,
                in const float initial_dist) {
	vec4 acc = vec4(0);

	float dist = initial_dist, inv_voxel_size = 1.0 / voxel_size;

	ivec3 axis_indices = ivec3(dir.x < 0.0 ? 0 : 1, dir.y < 0.0 ? 2 : 3, dir.z < 0.0 ? 4 : 5);
	vec3 axis_weights = dir * dir;

	while (dist < 4. && acc.a < 1.) {
		float diameter = 2. * tan_half_cone * dist;
		vec4 samp = sample_voxel(origin + dist * dir, log2(diameter * inv_voxel_size), axis_indices, axis_weights);
		acc += samp * (1.0 - acc.a);
		dist += diameter * 0.5;
	}

	return acc.rgb;
}

vec3 IndirectLight(in const vec3 position, in const vec3 normal) {
	mat3 tbn = normal_to_tbn(normal);

	float voxel_size = 2.0 / (VOXEL_SCALE * textureSize(uVoxelRadiance, 0).x);

	vec3 radiance = vec3(0);
	radiance += kConeWeights[0] * cone_trace(position, tbn * kConeDirections[0], 0.57735, voxel_size, 0.16);
	radiance += kConeWeights[1] * cone_trace(position, tbn * kConeDirections[1], 0.57735, voxel_size, 0.16);
	radiance += kConeWeights[2] * cone_trace(position, tbn * kConeDirections[2], 0.57735, voxel_size, 0.16);
	radiance += kConeWeights[3] * cone_trace(position, tbn * kConeDirections[3], 0.57735, voxel_size, 0.16);
	radiance += kConeWeights[4] * cone_trace(position, tbn * kConeDirections[4], 0.57735, voxel_size, 0.16);
	radiance += kConeWeights[5] * cone_trace(position, tbn * kConeDirections[5], 0.57735, voxel_size, 0.16);

	return radiance;
}

void main() {
	ivec2 coord = ivec2(gl_FragCoord.xy);
	vec3 albedo = texelFetch(uAlbedo, coord, 0).rgb;
	vec3 normal = normalize(oct_to_float32x3(texelFetch(uNormal, coord, 0).rg));
	float depth = texelFetch(uDepth, coord, 0).r;
	vec3 position = reconstruct_position(gl_FragCoord.xy, depth);

	vec3 light =
	    IsEmissive(albedo) ? albedo : albedo * IndirectLight(position, normal) * DirectVisibility(position, normal);
	light /= (light + 1);

	oLight = light;
}
