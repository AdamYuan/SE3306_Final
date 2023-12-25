#ifndef SHADER_TEXTURE_HPP
#define SHADER_TEXTURE_HPP

#ifdef GLSL
#ifndef TEXTURE_SET
#error TEXTURE_SET not defined
#endif
layout(set = TEXTURE_SET, binding = 0) uniform sampler2D uTumblerTexture;
layout(set = TEXTURE_SET, binding = 1) uniform sampler2D uFloorTexture;

vec3 texture_func_1(in const vec2 coord) { return texture(uFloorTexture, -coord.yx * .5 + .5).rgb; }
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
