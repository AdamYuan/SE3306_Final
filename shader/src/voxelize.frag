#version 450 core

#include "Config.h"

layout(location = 0) in vec3 gNormal;
layout(location = 1) in vec3 gColor;
layout(location = 2) in vec3 gWorldPos;
layout(location = 3) in vec4 gShadowPos;

layout(set = 0, rgba16f, binding = 0) uniform writeonly image3D uVoxelRadiance;
layout(set = 0, binding = 1) uniform sampler2DShadow uShadowMap;

#define TEXTURE_SET 1
#include "Texture.h"

ivec3 GetVoxelPos() {
	int voxel_resolution = imageSize(uVoxelRadiance).x;
	vec3 v = gWorldPos * VOXEL_SCALE * .5 + .5;
	v *= voxel_resolution;
	ivec3 u = ivec3(v);
	ivec2 bound_min_2 = ivec2((-VOXEL_SCALE * .5 + .5) * voxel_resolution + 1, 0);
	ivec2 bound_max_2 = ivec2((VOXEL_SCALE * .5 + .5) * voxel_resolution - 1, voxel_resolution - 1);
	ivec3 bound_min = IsTexture(gColor) ? bound_min_2.xyx : (IsEmissive(gColor) ? bound_min_2.xxx : bound_min_2.yyy);
	ivec3 bound_max = IsTexture(gColor) ? bound_max_2.xyx : (IsEmissive(gColor) ? bound_max_2.xxx : bound_max_2.yyy);
	u = clamp(u, bound_min, bound_max);
	return u;
}

void main() {
	ivec3 voxel_pos = GetVoxelPos();
	vec3 normal = normalize(gNormal);

	vec3 light_dir = normalize(vec3(0, kCornellLightHeight, 0) - gWorldPos);
	vec3 albedo = GetAlbedo(gColor);

	vec3 shadow_pos = gShadowPos.xyz / gShadowPos.w;
	shadow_pos.xy = shadow_pos.xy * 0.5 + 0.5;

	vec3 radiance = IsEmissive(albedo) ? albedo
	                                   : kCornellLightRadiance * albedo *
	                                         GetCornellLightVisibility(normal, GetCornellLightDir(gWorldPos),
	                                                                   textureProj(uShadowMap, vec4(shadow_pos, 1)));
	imageStore(uVoxelRadiance, voxel_pos, vec4(radiance, 1.));
}
