#version 450

#include "Binding.h"

layout(location = 0) uniform int uFirst;
layout(location = 1) uniform vec2 uJitter;

layout(location = 0) out vec3 oColor;

layout(binding = GBUFFER_PREV_UV_TEXTURE) uniform sampler2D uPrevUV;
layout(binding = TAA_TEXTURE) uniform sampler2D uPrevLight;
layout(binding = LIGHT_TEXTURE) uniform sampler2D uLight;

// note: clips towards aabb center
vec3 clip_aabb(in const vec3 aabb_min, // cn_min
               in const vec3 aabb_max, // cn_max
               in const vec3 p,        // c_in’
               in const vec3 q)        // c_hist
{
	vec3 p_clip = 0.5 * (aabb_max + aabb_min);
	vec3 e_clip = 0.5 * (aabb_max - aabb_min);
	vec3 v_clip = q - p_clip;
	vec3 v_unit = v_clip / e_clip;
	vec3 a_unit = abs(v_unit);
	float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));
	return ma_unit > 1.0 ? p_clip + v_clip / ma_unit : q;
}

const mat3 kRGB2YCoCg = mat3(0.25, 0.5, -0.25, 0.5, 0.0, 0.5, 0.25, -0.5, -0.25);
const mat3 kYCoCg2RGB = mat3(1.0, 1.0, 1.0, 1.0, 0.0, -1.0, -1.0, 1.0, -1.0);

void main() {
	ivec2 coord = ivec2(gl_FragCoord.xy), resolution = textureSize(uLight, 0);
	vec2 inv_resolution = 1.0 / resolution;
	vec2 uv = gl_FragCoord.xy * inv_resolution, uv_unjitter = uv + uJitter * .5;

	vec3 light = texture(uLight, uv_unjitter).rgb;

	if (uFirst == 1)
		oColor = light;
	else {
		vec2 prev_uv = texelFetch(uPrevUV, coord, 0).rg + uJitter * .5;
		vec3 prev_light = kRGB2YCoCg * texture(uPrevLight, prev_uv).rgb;

		float dx = inv_resolution.x, dy = inv_resolution.y;
		vec3 l0 = kRGB2YCoCg * texture(uLight, vec2(uv_unjitter.x - dx, uv_unjitter.y)).rgb;
		vec3 l1 = kRGB2YCoCg * texture(uLight, vec2(uv_unjitter.x + dx, uv_unjitter.y)).rgb;
		vec3 l2 = kRGB2YCoCg * texture(uLight, vec2(uv_unjitter.x, uv_unjitter.y - dy)).rgb;
		vec3 l3 = kRGB2YCoCg * texture(uLight, vec2(uv_unjitter.x, uv_unjitter.y + dy)).rgb;

		light = kRGB2YCoCg * light;
		vec3 l_min = min(min(l0, l1), min(l2, min(l3, light)));
		vec3 l_max = max(max(l0, l1), max(l2, max(l3, light)));

		// prev_light = clamp(prev_light, l_min, l_max);
		prev_light = clip_aabb(l_min, l_max, light, prev_light);

		oColor = kYCoCg2RGB * mix(light, prev_light, 0.65);
	}
}
