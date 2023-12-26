#version 450

layout(location = 0) out vec2 oVelocity;

layout(binding = 0) uniform sampler2D uTile0;

vec2 reduce(in const vec2 l, in const vec2 r) { return dot(l, l) >= dot(r, r) ? l : r; }

void main() {
	vec2 inv_resolution = 1.0 / textureSize(uTile0, 0);
	vec2 uv = gl_FragCoord.xy * inv_resolution;
	float dx = inv_resolution.x, dy = inv_resolution.y;

	vec2 s0 = texture(uTile0, vec2(uv.x - dx, uv.y - dy)).rg;
	vec2 s1 = texture(uTile0, vec2(uv.x, uv.y - dy)).rg;
	vec2 s2 = texture(uTile0, vec2(uv.x + dx, uv.y - dy)).rg;
	vec2 s3 = texture(uTile0, vec2(uv.x - dx, uv.y)).rg;
	vec2 s4 = texture(uTile0, vec2(uv.x, uv.y)).rg;
	vec2 s5 = texture(uTile0, vec2(uv.x + dx, uv.y)).rg;
	vec2 s6 = texture(uTile0, vec2(uv.x - dx, uv.y + dy)).rg;
	vec2 s7 = texture(uTile0, vec2(uv.x, uv.y + dy)).rg;
	vec2 s8 = texture(uTile0, vec2(uv.x + dx, uv.y + dy)).rg;

	oVelocity = reduce(reduce(reduce(reduce(s0, s1), reduce(s2, s3)), reduce(reduce(s4, s5), reduce(s6, s7))), s8);
}
