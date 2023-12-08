#version 450

#include "Binding.h"

layout(location = 0) out vec4 oColor;

layout(binding = BLOOM_0_TEXTURE) uniform sampler2D uBloom0;

vec4 blur5_v(in const vec2 uv, in const vec2 inv_resolution) {
	vec2 off = vec2(0, 1.3333333333333333) * inv_resolution;
	vec4 color = vec4(0.0);
	color += texture(uBloom0, uv) * 0.29411764705882354;
	color += texture(uBloom0, uv + off) * 0.35294117647058826;
	color += texture(uBloom0, uv - off) * 0.35294117647058826;
	return color;
}

vec4 blur9_v(in const vec2 uv, in const vec2 inv_resolution) {
	vec4 color = vec4(0.0);
	vec2 off1 = vec2(0, 1.3846153846) * inv_resolution;
	vec2 off2 = vec2(0, 3.2307692308) * inv_resolution;
	color += texture(uBloom0, uv) * 0.2270270270;
	color += texture(uBloom0, uv + off1) * 0.3162162162;
	color += texture(uBloom0, uv - off1) * 0.3162162162;
	color += texture(uBloom0, uv + off2) * 0.0702702703;
	color += texture(uBloom0, uv - off2) * 0.0702702703;
	return color;
}

vec4 blur13_v(in const vec2 uv, in const vec2 inv_resolution) {
	vec4 color = vec4(0.0);
	vec2 off1 = vec2(0, 1.411764705882353) * inv_resolution;
	vec2 off2 = vec2(0, 3.2941176470588234) * inv_resolution;
	vec2 off3 = vec2(0, 5.176470588235294) * inv_resolution;
	color += texture(uBloom0, uv) * 0.1964825501511404;
	color += texture(uBloom0, uv + off1) * 0.2969069646728344;
	color += texture(uBloom0, uv - off1) * 0.2969069646728344;
	color += texture(uBloom0, uv + off2) * 0.09447039785044732;
	color += texture(uBloom0, uv - off2) * 0.09447039785044732;
	color += texture(uBloom0, uv + off3) * 0.010381362401148057;
	color += texture(uBloom0, uv - off3) * 0.010381362401148057;
	return color;
}

void main() {
	vec2 resolution = vec2(textureSize(uBloom0, 0));
	vec2 inv_resolution = 1. / resolution;
	oColor = blur9_v(gl_FragCoord.xy * inv_resolution, inv_resolution);
}
