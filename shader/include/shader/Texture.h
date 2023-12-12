#ifndef SHADER_TEXTURE_HPP
#define SHADER_TEXTURE_HPP

#include "Binding.h"

layout(binding = TUMBLER_TEXTURE) uniform sampler2D uTumblerTexture;
layout(binding = FLOOR_TEXTURE) uniform sampler2D uFloorTexture;

#ifdef GLSL
vec3 texture_func_1(in const vec2 coord) {
	return texture(uFloorTexture, -coord.yx * .5 + .5).rgb;
	/* const vec3 color = vec3(.725, .71, .68);
	const float coef[2] = {.9, .5};
	ivec2 icoord = ivec2(floor(coord * 4));
	return color * coef[(icoord.x + icoord.y) & 1]; */
}
vec3 texture_func_2(in const vec2 coord) {
	const vec3 bg = vec3(.63, .065, .05);
	vec4 samp = texture(uTumblerTexture, vec2(coord.x * 1.5, 1. - coord.y));
	return mix(bg, samp.rgb, samp.a);
}
bool IsTexture(in const vec3 color) { return color.r < 0; }
vec3 GetAlbedo(in const vec3 color) {
	return color.r >= .0 ? color : (color.r < -1.5 ? texture_func_2(color.yz) : texture_func_1(color.yz));
}
#endif

#endif
