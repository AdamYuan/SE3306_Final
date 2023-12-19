#version 450

#include "Binding.h"

layout(location = 0) uniform int uFirst;
layout(location = 0) out vec3 oColor;

layout(binding = GBUFFER_PREV_UV_TEXTURE) uniform sampler2D uPrevUV;
layout(binding = TAA_TEXTURE) uniform sampler2D uPrevLight;
layout(binding = LIGHT_TEXTURE) uniform sampler2D uLight;

// note: clips towards aabb center + p.w
vec4 clip_aabb(in const vec3 aabb_min, // cn_min
               in const vec3 aabb_max, // cn_max
               in const vec4 p,        // c_in’
               in const vec4 q)        // c_hist
{
	vec3 p_clip = 0.5 * (aabb_max + aabb_min);
	vec3 e_clip = 0.5 * (aabb_max - aabb_min);
	vec4 v_clip = q - vec4(p_clip, p.w);
	vec3 v_unit = v_clip.xyz / e_clip;
	vec3 a_unit = abs(v_unit);
	float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));
	if (ma_unit > 1.0)
		return vec4(p_clip, p.w) + v_clip / ma_unit;
	else
		return q; // point inside aabb
}

void main() {
	ivec2 coord = ivec2(gl_FragCoord.xy), resolution = textureSize(uLight, 0);
	vec2 inv_resolution = 1.0 / resolution, uv = gl_FragCoord.xy * inv_resolution;

	vec3 light = texelFetch(uLight, coord, 0).rgb;

	if (uFirst == 1)
		oColor = light;
	else {
		vec2 prev_uv = texelFetch(uPrevUV, coord, 0).rg;
		vec3 prev_light = texture(uPrevLight, prev_uv).rgb;

		vec3 l0 = texture(uLight, vec2(uv.x - inv_resolution.x, uv.y)).rgb;
		vec3 l1 = texture(uLight, vec2(uv.x + inv_resolution.x, uv.y)).rgb;
		vec3 l2 = texture(uLight, vec2(uv.x, uv.y - inv_resolution.y)).rgb;
		vec3 l3 = texture(uLight, vec2(uv.x, uv.y + inv_resolution.y)).rgb;

		vec3 l_min = min(min(l0, l1), min(l2, min(l3, light)));
		vec3 l_max = max(max(l0, l1), max(l2, max(l3, light)));

		prev_light = clamp(prev_light, l_min, l_max);

		oColor = mix(prev_light, light, 0.2);
	}
}
