#version 450

layout(location = 0) out float oOcclusion;

layout(binding = 1) uniform sampler2D uNormal;
layout(binding = 2) uniform sampler2D uDepth;
layout(binding = 4) uniform sampler1D uSamples;
layout(binding = 5) uniform sampler2D uNoise;

layout(std140, binding = 0) uniform uuCamera { mat4 uViewProjection, uInverseViewProjection, uShadowViewProjection; };

vec2 sign_not_zero(in const vec2 v) { return vec2((v.x >= 0.0) ? 1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0); }
vec3 oct_to_float32x3(vec2 e) {
	vec3 v = vec3(e.xy, 1.0 - abs(e.x) - abs(e.y));
	if (v.z < 0)
		v.xy = (1.0 - abs(v.yx)) * sign_not_zero(v.xy);
	return normalize(v);
}

vec3 reconstruct_position(in const vec2 frag_coord, in float depth) {
	vec4 clip = vec4((frag_coord / textureSize(uDepth, 0).xy) * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
	vec4 rec = uInverseViewProjection * clip;
	return rec.xyz / rec.w;
}

const float kRadius = 0.1;

// const float kNear = 0.1, kFar = 32.0;
// float linearize_depth(in const float depth) { return (2.0 * kNear * kFar) / (kFar + kNear - depth * (kFar - kNear));
// }

void main() {
	ivec2 coord = ivec2(gl_FragCoord.xy);
	vec3 normal = oct_to_float32x3(texelFetch(uNormal, coord, 0).rg);
	float depth = texelFetch(uDepth, coord, 0).r;
	if (depth == 1.0) {
		oOcclusion = 1.0;
		return;
	}
	vec3 position = reconstruct_position(gl_FragCoord.xy, depth);

	vec3 noise = vec3(texelFetch(uNoise, coord & 0x3, 0).xy, 0);
	vec3 tangent = normalize(noise - normal * dot(noise, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 tbn_matrix = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for (int i = 0; i < 64; ++i) {
		vec3 sample_pos = position + tbn_matrix * texelFetch(uSamples, i, 0).xyz * kRadius;

		vec4 sample_clip = uViewProjection * vec4(sample_pos, 1.0);
		sample_clip.xyz /= sample_clip.w;

		float real_depth = texture(uDepth, sample_clip.xy * 0.5 + 0.5).r, sample_depth = sample_clip.z * 0.5 + 0.5;

		occlusion += float(real_depth >= sample_depth);
	}

	oOcclusion = occlusion * 0.015625;
}
