#ifndef SHADER_TEXTURE_HPP
#define SHADER_TEXTURE_HPP

#include "Binding.h"

layout(binding = TUMBLER_TEXTURE) uniform sampler2D uTumblerTexture;

#ifdef GLSL
vec3 texture_func_1(in const vec2 coord) {
	const vec3 color = vec3(.725, .71, .68);
	const float coef[2] = {1.1, 0.5};
	ivec2 icoord = ivec2(floor(coord * 4));
	return color * coef[(icoord.x + icoord.y) & 1];
}
vec3 texture_func_2(in const vec2 coord) {
	const vec3 bg = vec3(.63, .065, .05);
	vec4 samp = texture(uTumblerTexture, vec2(coord.x * 1.5, 1. - coord.y));
	return mix(bg, samp.rgb, samp.a);
}
vec3 GetAlbedo(in const vec3 color) {
	return color.r < -1.5 ? texture_func_2(color.yz) : (color.r < .0 ? texture_func_1(color.yz) : color);
}
#endif

#endif
