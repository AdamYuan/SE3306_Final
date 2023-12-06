#ifndef SHADER_TEXTURE_HPP
#define SHADER_TEXTURE_HPP

#ifdef GLSL
vec3 texture_func_1(in const vec2 coord) {
	const vec3 color = vec3(.725, .71, .68);
	int id = int(floor(coord.x * 4)) + int(floor(coord.y * 4));
	return (id & 1) == 0 ? color * 1.1 : color * .5;
}
vec3 GetAlbedo(in const vec3 color) { return color.r < .0 ? texture_func_1(color.yz) : color; }
#endif

#endif
