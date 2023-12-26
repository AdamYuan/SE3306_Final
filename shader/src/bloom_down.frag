#version 450

// https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom
//
// This shader performs downsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.
// This particular method was customly designed to eliminate
// "pulsating artifacts and temporal stability issues".

// Remember to add bilinear minification filter for this texture!
// Remember to use a floating-point texture format (for HDR)!
// Remember to use edge clamping for this texture!

layout(binding = 0) uniform sampler2D uBloom;
layout(location = 0) out vec4 oDownSample;

void main() {
	ivec2 src_resolution = textureSize(uBloom, 0), dst_resolution = src_resolution >> 1;
	vec2 texcoord = gl_FragCoord.xy / vec2(dst_resolution);
	vec2 src_texel_size = 1.0 / vec2(src_resolution);
	float x = src_texel_size.x;
	float y = src_texel_size.y;

	// Take 13 samples around current texel:
	// a - b - c
	// - j - k -
	// d - e - f
	// - l - m -
	// g - h - i
	// === ('e' is the current texel) ===
	vec3 a = texture(uBloom, vec2(texcoord.x - 2 * x, texcoord.y + 2 * y)).rgb;
	vec3 b = texture(uBloom, vec2(texcoord.x, texcoord.y + 2 * y)).rgb;
	vec3 c = texture(uBloom, vec2(texcoord.x + 2 * x, texcoord.y + 2 * y)).rgb;

	vec3 d = texture(uBloom, vec2(texcoord.x - 2 * x, texcoord.y)).rgb;
	vec3 e = texture(uBloom, vec2(texcoord.x, texcoord.y)).rgb;
	vec3 f = texture(uBloom, vec2(texcoord.x + 2 * x, texcoord.y)).rgb;

	vec3 g = texture(uBloom, vec2(texcoord.x - 2 * x, texcoord.y - 2 * y)).rgb;
	vec3 h = texture(uBloom, vec2(texcoord.x, texcoord.y - 2 * y)).rgb;
	vec3 i = texture(uBloom, vec2(texcoord.x + 2 * x, texcoord.y - 2 * y)).rgb;

	vec3 j = texture(uBloom, vec2(texcoord.x - x, texcoord.y + y)).rgb;
	vec3 k = texture(uBloom, vec2(texcoord.x + x, texcoord.y + y)).rgb;
	vec3 l = texture(uBloom, vec2(texcoord.x - x, texcoord.y - y)).rgb;
	vec3 m = texture(uBloom, vec2(texcoord.x + x, texcoord.y - y)).rgb;

	// Apply weighted distribution:
	// 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
	// a,b,d,e * 0.125
	// b,c,e,f * 0.125
	// d,e,g,h * 0.125
	// e,f,h,i * 0.125
	// j,k,l,m * 0.5
	// This shows 5 square areas that are being sampled. But some of them overlap,
	// so to have an energy preserving downsample we need to make some adjustments.
	// The weights are the distributed, so that the sum of j,k,l,m (e.g.)
	// contribute 0.5 to the final color output. The code below is written
	// to effectively yield this sum. We get:
	// 0.125*5 + 0.03125*4 + 0.0625*4 = 1
	vec3 down = e * 0.125;
	down += (a + c + g + i) * 0.03125;
	down += (b + d + f + h) * 0.0625;
	down += (j + k + l + m) * 0.125;
	oDownSample = vec4(down, 1.0);
}
