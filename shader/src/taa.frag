#version 450

#include "Binding.h"

layout(location = 0) uniform int uFirst;
layout(location = 1) uniform vec2 uJitter;

layout(location = 0) out vec3 oColor;

layout(binding = GBUFFER_VELOCITY_TEXTURE) uniform sampler2D uVelocity;
layout(binding = TAA_TEXTURE) uniform sampler2D uPrevLight;
layout(binding = LIGHT_TEXTURE) uniform sampler2D uLight;

vec3 VarianceClip(in const vec3 q, in const vec3 mean, in const vec3 stddev) {
	vec3 v_clip = q - mean;
	vec3 v_unit = v_clip / stddev;
	vec3 a_unit = abs(v_unit);
	float ma_unit = max(a_unit.x, max(a_unit.y, a_unit.z));
	return ma_unit > 1.0 ? mean + v_clip / ma_unit : q;
}

vec3 RGB2YCoCg(in const vec3 rgb) {
	float co = rgb.r - rgb.b;
	float t = rgb.b + co / 2.0;
	float cg = rgb.g - t;
	float y = t + cg / 2.0;
	return vec3(y, co, cg);
}

vec3 YCoCg2RGB(in const vec3 ycocg) {
	float t = ycocg.r - ycocg.b / 2.0;
	float g = ycocg.b + t;
	float b = t - ycocg.g / 2.0;
	float r = ycocg.g + b;
	return vec3(r, g, b);
}

void main() {
	ivec2 coord = ivec2(gl_FragCoord.xy), resolution = textureSize(uLight, 0);
	vec2 inv_resolution = 1.0 / resolution;
	vec2 uv = gl_FragCoord.xy * inv_resolution, uv_unjitter = uv + uJitter * .5;

	vec3 light = texture(uLight, uv_unjitter).rgb;

	if (uFirst == 1)
		oColor = light;
	else {
		vec2 velocity = texelFetch(uVelocity, coord, 0).rg - .5;
		vec3 prev_light = RGB2YCoCg(texture(uPrevLight, uv - velocity).rgb);

		light = RGB2YCoCg(light);
		vec3 mean = light, stddev = light * light;
		float dx = inv_resolution.x, dy = inv_resolution.y;
#define ACC(UV) \
	{ \
		vec3 l = RGB2YCoCg(texture(uLight, UV).rgb); \
		mean += l; \
		stddev += l * l; \
	}
		ACC(vec2(uv_unjitter.x - dx, uv_unjitter.y));
		ACC(vec2(uv_unjitter.x + dx, uv_unjitter.y));
		ACC(vec2(uv_unjitter.x, uv_unjitter.y - dy));
		ACC(vec2(uv_unjitter.x, uv_unjitter.y + dy));

		mean *= 0.2;
		stddev = sqrt(max(stddev * 0.2 - mean * mean, 1e-12));

		prev_light = VarianceClip(prev_light, mean, stddev);

		oColor = YCoCg2RGB(mix(light, prev_light, 0.7));
	}
}
