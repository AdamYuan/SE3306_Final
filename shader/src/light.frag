#version 450

#include "Config.h"

const uint uTick = 0;

layout(location = 0) out vec4 oLight;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput uAlbedo;
// layout(input_attachment_index = 1, binding = 1) uniform subpassInput uNormal;
// layout(binding = 0) uniform sampler2D uAlbedo;
layout(binding = 1) uniform sampler2D uNormal;
layout(binding = 2) uniform sampler2D uDepth;
layout(binding = 3) uniform sampler2DShadow uShadowMap;
layout(binding = 4) uniform sampler3D uVoxelRadiance;
layout(binding = 5) uniform sampler3D uVoxelRadianceMipmaps[6];

layout(push_constant) uniform uuPushConstant { mat4 uInvViewProj, uShadowViewProj; };

vec3 reconstruct_position(in const vec2 frag_coord, in float depth) {
	vec4 clip = vec4((frag_coord / textureSize(uDepth, 0).xy) * 2.0 - 1.0, depth, 1.0);
	clip.y = -clip.y;
	vec4 rec = uInvViewProj * clip;
	return rec.xyz / rec.w;
}

vec2 sign_not_zero(in const vec2 v) { return vec2((v.x >= 0.0) ? 1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0); }
vec3 oct_to_float32x3(vec2 e) {
	vec3 v = vec3(e.xy, 1.0 - abs(e.x) - abs(e.y));
	if (v.z < 0)
		v.xy = (1.0 - abs(v.yx)) * sign_not_zero(v.xy);
	return normalize(v);
}

float InterleavedGradientNoise(in const ivec2 pixel_pos) {
	return fract(52.9829189 * fract(dot(vec2(pixel_pos), vec2(0.06711056, 0.00583715))));
}
float DirectVisibility(in const vec3 position, in const vec3 normal) {
	vec3 light_dir = GetCornellLightDir(position);
	vec4 shadow_pos = uShadowViewProj * vec4(position, 1);
	shadow_pos.y = -shadow_pos.y;
	shadow_pos /= shadow_pos.w;
	shadow_pos.xy = shadow_pos.xy * 0.5 + 0.5;

	float shadow = 0;
	const vec2 kPoissonDisk[16] =
	    vec2[](vec2(-0.94201624, -0.39906216), vec2(0.94558609, -0.76890725), vec2(-0.094184101, -0.92938870),
	           vec2(0.34495938, 0.29387760), vec2(-0.91588581, 0.45771432), vec2(-0.81544232, -0.87912464),
	           vec2(-0.38277543, 0.27676845), vec2(0.97484398, 0.75648379), vec2(0.44323325, -0.97511554),
	           vec2(0.53742981, -0.47373420), vec2(-0.26496911, -0.41893023), vec2(0.79197514, 0.19090188),
	           vec2(-0.24188840, 0.99706507), vec2(-0.81409955, 0.91437590), vec2(0.19984126, 0.78641367),
	           vec2(0.14383161, -0.14100790));
	vec2 unit = 1.5 / textureSize(uShadowMap, 0);
	uint idx = (uint(16.0 * InterleavedGradientNoise(ivec2(gl_FragCoord.xy))) + uTick) & 0xfu;
	shadow = textureProj(uShadowMap, vec4(shadow_pos.xy + kPoissonDisk[idx] * unit, shadow_pos.zw));
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

	while (dist < 4. && acc.a < .99) {
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

// from https://www.shadertoy.com/view/7ltGz7, for presentation
vec3 VoxelRayMarch(sampler3D voxels, in const int lod, in const vec3 origin, in const vec3 dir) {
	ivec3 voxel_dim = textureSize(voxels, lod);
	float voxel_size = 2.0 / (VOXEL_SCALE * voxel_dim.x);

	vec3 position = (origin * VOXEL_SCALE * .5 + .5) * vec3(voxel_dim);
	vec3 voxel_pos = floor(position);

	vec3 step_dir = sign(dir);

	vec3 delta = voxel_size / dir;

	vec3 max_dist = vec3(step_dir.x > 0.0 ? voxel_pos.x + 1.0 - position.x : position.x - voxel_pos.x,
	                     step_dir.y > 0.0 ? voxel_pos.y + 1.0 - position.y : position.y - voxel_pos.y,
	                     step_dir.z > 0.0 ? voxel_pos.z + 1.0 - position.z : position.z - voxel_pos.z);

	max_dist *= delta;

	for (int i = 0; i < 256; i++) {
		vec4 samp = texelFetch(voxels, clamp(ivec3(voxel_pos), ivec3(0), voxel_dim - 1), lod);
		if (samp.a > 0)
			return samp.rgb;

		vec3 abs_max = abs(max_dist);

		if (abs_max.x < abs_max.y && abs_max.x < abs_max.z) {
			voxel_pos.x += step_dir.x;
			max_dist.x += delta.x;
		} else if (abs_max.y < abs_max.z) {
			voxel_pos.y += step_dir.y;
			max_dist.y += delta.y;
		} else {
			voxel_pos.z += step_dir.z;
			max_dist.z += delta.z;
		}
	}
	return vec3(0);
}

void main() {
	ivec2 coord = ivec2(gl_FragCoord.xy);
	// vec3 albedo = texelFetch(uAlbedo, coord, 0).rgb;
	vec3 normal = normalize(oct_to_float32x3(texelFetch(uNormal, coord, 0).rg));
	vec3 albedo = subpassLoad(uAlbedo).rgb;
	// vec3 normal = normalize(oct_to_float32x3(subpassLoad(uNormal).rg));
	float depth = texelFetch(uDepth, coord, 0).r;
	vec3 position = reconstruct_position(gl_FragCoord.xy, depth);

	/* {
	    vec3 origin = vec3(0, 0, 1 + sqrt(3.));
	    vec3 dir = normalize(position - origin);
	    vec3 color = VoxelRayMarch(uVoxelRadianceMipmaps[0], 0, origin, dir);
	    color /= color + 1;
	    oLight = vec4(color, 1.0);
	    return;
	} */

	vec3 light =
	    IsEmissive(albedo) ? albedo : albedo * IndirectLight(position, normal) * DirectVisibility(position, normal);
	light /= (light + 1);

	oLight = vec4(light, 1.0);
}
