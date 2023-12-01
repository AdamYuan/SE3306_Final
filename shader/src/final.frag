#version 450

layout(location = 0) out vec4 oColor;

layout(binding = 0) uniform sampler2D uAlbedo;
layout(binding = 1) uniform sampler2D uNormal;
layout(binding = 3) uniform sampler2D uOcclusion;

vec2 sign_not_zero(in const vec2 v) { return vec2((v.x >= 0.0) ? 1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0); }
vec3 oct_to_float32x3(vec2 e) {
	vec3 v = vec3(e.xy, 1.0 - abs(e.x) - abs(e.y));
	if (v.z < 0)
		v.xy = (1.0 - abs(v.yx)) * sign_not_zero(v.xy);
	return normalize(v);
}

void main() {
	ivec2 coord = ivec2(gl_FragCoord.xy);
	vec3 albedo = texelFetch(uAlbedo, coord, 0).rgb;
	vec3 normal = oct_to_float32x3(texelFetch(uNormal, coord, 0).rg);

	float occlusion;
	{
		vec2 ps = 1.0 / vec2(textureSize(uOcclusion, 0)).xy;
		vec2 uv = gl_FragCoord.xy * ps;
		uint cnt = 0;

#define BLUR(NEI_UV) \
	{ \
		vec2 nei_uv = NEI_UV; \
		if (texture(uAlbedo, nei_uv).rgb == albedo) { \
			++cnt; \
			occlusion += texture(uOcclusion, nei_uv).r; \
		} \
	}
		BLUR(vec2(uv.x - ps.x, uv.y - ps.y))
		BLUR(vec2(uv.x, uv.y - ps.y))
		BLUR(vec2(uv.x + ps.x, uv.y - ps.y))
		BLUR(vec2(uv.x - ps.x, uv.y))
		BLUR(uv)
		BLUR(vec2(uv.x + ps.x, uv.y))
		BLUR(vec2(uv.x - ps.x, uv.y + ps.y))
		BLUR(vec2(uv.x, uv.y + ps.y))
		BLUR(vec2(uv.x + ps.x, uv.y + ps.y))
		occlusion /= float(cnt);
#undef BLUR
	}

	float diffuse = max(dot(normal, normalize(vec3(3, 6, 2))), 0.0) * 0.3;
	vec3 color = albedo * (diffuse + 0.7 * occlusion);
	oColor = vec4(pow(color, vec3(1.0 / 1.5)), 1.0);
}
