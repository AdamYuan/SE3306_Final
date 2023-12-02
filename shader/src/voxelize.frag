#version 450 core

#include "Binding.h"

layout(location = 0) in vec3 gNormal;
layout(location = 1) in vec3 gAlbedo;
layout(location = 2) in vec3 gWorldPos;
layout(location = 3) in vec3 gShadowPos;
layout(location = 4) flat in uint gAxis;

layout(rgba8, binding = VOXEL_ALBEDO_IMAGE) uniform writeonly image3D uVoxelAlbedo;
layout(rg8_snorm, binding = VOXEL_NORMAL_IMAGE) uniform writeonly image3D uVoxelNormal;
layout(rgba16f, binding = VOXEL_RADIANCE_IMAGE) uniform writeonly image3D uVoxelRadiance;

layout(binding = SHADOW_MAP_TEXTURE) uniform sampler2DShadow uShadowMap;

ivec3 GetVoxePos() {
	int voxel_resolution = imageSize(uVoxelAlbedo).x;
	vec3 v = gl_FragCoord.xyz;
	v.z *= float(voxel_resolution);
	ivec3 u = clamp(ivec3(v), ivec3(0u), ivec3(voxel_resolution - 1));
	return gAxis == 0 ? u.zxy : (gAxis == 1 ? u.yzx : u.xyz);
}

vec2 sign_not_zero(in const vec2 v) { return vec2((v.x >= 0.0) ? 1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0); }
// Assume normalized input. Output is on [-1, 1] for each component.
vec2 float32x3_to_oct(in const vec3 v) {
	// Project the sphere onto the octahedron, and then onto the xy plane
	vec2 p = v.xy * (1.0 / (abs(v.x) + abs(v.y) + abs(v.z)));
	// Reflect the folds of the lower hemisphere over the diagonals
	return (v.z <= 0.0) ? ((1.0 - abs(p.yx)) * sign_not_zero(p)) : p;
}

const float kCornellLightHeight = 1.5;
const vec3 kCornellLightRadiance = vec3(2.0);

void main() {
	ivec3 voxel_pos = GetVoxePos();
	imageStore(uVoxelNormal, voxel_pos, vec4(float32x3_to_oct(gNormal), .0, .0));
	imageStore(uVoxelAlbedo, voxel_pos, vec4(gAlbedo, 1.));

	vec3 light_dir = normalize(vec3(0, kCornellLightHeight, 0) - gWorldPos);
	float visibility =
	    gAlbedo == vec3(1) ? 1.0 : max(dot(gNormal, light_dir), 0.0) * textureProj(uShadowMap, vec4(gShadowPos, 1));
	vec3 radiance = kCornellLightRadiance * gAlbedo * visibility;
	imageStore(uVoxelRadiance, voxel_pos, vec4(radiance, 1.));
}
