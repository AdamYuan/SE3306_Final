#version 450

#include "Binding.h"
#include "Texture.h"

layout(location = 0) out vec3 oColor;

layout(binding = GBUFFER_ALBEDO_TEXTURE) uniform sampler2D uAlbedo;

vec4 sample_color(in const vec2 uv) {
	vec3 color = texture(uAlbedo, uv).rgb;
	return any(greaterThan(color.rgb, vec3(1))) ? vec4(color, 1) : vec4(0);
}

void main() {
	vec3 albedo = texelFetch(uAlbedo, ivec2(gl_FragCoord.xy), 0).rgb;
	oColor = IsEmissive(albedo) ? albedo : vec3(0);
}
