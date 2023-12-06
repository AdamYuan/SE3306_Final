#version 450 core

#include "Binding.h"
#include "Config.h"
#include "Texture.h"

layout(location = 0) in vec3 gNormal;
layout(location = 1) in vec3 gColor;
layout(location = 2) in vec3 gWorldPos;
layout(location = 3) in vec3 gShadowPos;
layout(location = 4) flat in uint gAxis;

layout(rgba16f, binding = VOXEL_RADIANCE_IMAGE) uniform writeonly image3D uVoxelRadiance;
layout(binding = SHADOW_MAP_TEXTURE) uniform sampler2DShadow uShadowMap;

ivec3 GetVoxePos() {
	int voxel_resolution = imageSize(uVoxelRadiance).x;
	vec3 v = gl_FragCoord.xyz;
	v.z *= float(voxel_resolution);
	ivec3 u = clamp(ivec3(v), ivec3(0u), ivec3(voxel_resolution - 1));
	return gAxis == 0 ? u.zxy : (gAxis == 1 ? u.yzx : u.xyz);
}

void main() {
	ivec3 voxel_pos = GetVoxePos();
	vec3 normal = normalize(gNormal);

	vec3 light_dir = normalize(vec3(0, kCornellLightHeight, 0) - gWorldPos);
	vec3 albedo = GetAlbedo(gColor);

	bool emissive = any(greaterThan(albedo, vec3(1)));
	vec3 radiance = emissive ? albedo
	                         : kCornellLightRadiance * albedo * max(dot(normal, light_dir), 0.0) *
	                               textureProj(uShadowMap, vec4(gShadowPos, 1));
	imageStore(uVoxelRadiance, voxel_pos, vec4(radiance, 1.));
}
