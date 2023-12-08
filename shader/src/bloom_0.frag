#version 450

#include "Binding.h"

layout(location = 0) out vec4 oColor;

layout(binding = GBUFFER_ALBEDO_TEXTURE) uniform sampler2D uAlbedo;

vec4 sample_color(in const vec2 uv) {
	vec3 color = texture(uAlbedo, uv).rgb;
	return any(greaterThan(color.rgb, vec3(1))) ? vec4(color, 1) : vec4(0);
}

vec4 blur5_h(in const vec2 uv, in const vec2 inv_resolution) {
	vec2 off = vec2(1.3333333333333333, 0) * inv_resolution;
	vec4 color = vec4(0.0);
	color += sample_color(uv) * 0.29411764705882354;
	color += sample_color(uv + off) * 0.35294117647058826;
	color += sample_color(uv - off) * 0.35294117647058826;
	return color;
}

vec4 blur9_h(in const vec2 uv, in const vec2 inv_resolution) {
	vec4 color = vec4(0.0);
	vec2 off1 = vec2(1.3846153846, 0) * inv_resolution;
	vec2 off2 = vec2(3.2307692308, 0) * inv_resolution;
	color += sample_color(uv) * 0.2270270270;
	color += sample_color(uv + off1) * 0.3162162162;
	color += sample_color(uv - off1) * 0.3162162162;
	color += sample_color(uv + off2) * 0.0702702703;
	color += sample_color(uv - off2) * 0.0702702703;
	return color;
}

vec4 blur13_h(in const vec2 uv, in const vec2 inv_resolution) {
	vec4 color = vec4(0.0);
	vec2 off1 = vec2(1.411764705882353, 0) * inv_resolution;
	vec2 off2 = vec2(3.2941176470588234, 0) * inv_resolution;
	vec2 off3 = vec2(5.176470588235294, 0) * inv_resolution;
	color += sample_color(uv) * 0.1964825501511404;
	color += sample_color(uv + off1) * 0.2969069646728344;
	color += sample_color(uv - off1) * 0.2969069646728344;
	color += sample_color(uv + off2) * 0.09447039785044732;
	color += sample_color(uv - off2) * 0.09447039785044732;
	color += sample_color(uv + off3) * 0.010381362401148057;
	color += sample_color(uv - off3) * 0.010381362401148057;
	return color;
}

void main() {
	vec2 resolution = vec2(textureSize(uAlbedo, 0));
	vec2 inv_resolution = 1. / resolution;
	oColor = blur9_h(gl_FragCoord.xy * inv_resolution, inv_resolution);
}
