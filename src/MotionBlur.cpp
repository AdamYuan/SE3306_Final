#include "MotionBlur.hpp"

#include <shader/Binding.h>
#include <shader/Config.h>

#include <string>

inline static constexpr auto div_ceil(auto x, auto y) { return x / y + (x % y == 0 ? 0 : 1); }

void MotionBlur::initialize_tile_max_shader() {
	int subgroup_size = 1, shared_size = m_tile_size * m_tile_size;
	if (mygl3::Shader s; mygl3::IsExtensionSupported("GL_KHR_shader_subgroup") && //
	                     (s.Initialize(), s.Load(R"(#version 450
#extension GL_KHR_shader_subgroup_basic : require
#extension GL_KHR_shader_subgroup_ballot : require
#extension GL_KHR_shader_subgroup_arithmetic : require
void main(){})",
	                                             GL_COMPUTE_SHADER))) {
		glGetIntegerv(GL_SUBGROUP_SIZE_KHR, &subgroup_size);
		printf("GL_KHR_shader_subgroup_{basic, ballot, arithmetic} supported, gl_SubgroupSize = %d\n", subgroup_size);
		shared_size = div_ceil(shared_size, subgroup_size);
	} else
		printf("GL_KHR_shader_subgroup_{basic, ballot, arithmetic} not supported\n");

	std::string macro = "#define TILE_SIZE " + std::to_string(m_tile_size) + "\n";
	if (subgroup_size > 1)
		macro += "#define SUBGROUP_SIZE " + std::to_string(subgroup_size) + "\n";
	if (shared_size > 1)
		macro += "#define SHARED_SIZE " + std::to_string(shared_size) + "\n";

	m_tile_max_shader.Initialize();
	char comp[sizeof(
#include <shader/mb_tile_max.comp.str>
	              ) +
	          256];
	sprintf(comp,
#include <shader/mb_tile_max.comp.str>
	        , macro.c_str());
	m_tile_max_shader.Load(comp, GL_COMPUTE_SHADER);
	m_tile_max_shader.Finalize();
}

void MotionBlur::generate_tile_max() {
	m_tile_max_shader.Use();
	glDispatchCompute(div_ceil(m_width, m_tile_size), div_ceil(m_height, m_tile_size), 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void MotionBlur::Initialize(const char *quad_vert_str) {}

void MotionBlur::initialize_target(int width, int height, int tile_size) {
	if (m_width == width && m_height == height && m_tile_size == tile_size)
		return;
	m_width = width;
	m_height = height;

	if (m_tile_size != tile_size) {
		m_tile_size = tile_size;
		initialize_tile_max_shader();
	}

	m_tile_0.Initialize();
	m_tile_0.Storage(div_ceil(width, tile_size), div_ceil(height, tile_size), GL_RG16, 1);
	m_tile_0.SetSizeFilter(GL_LINEAR, GL_LINEAR);
	m_tile_0.SetWrapFilter(GL_CLAMP_TO_EDGE);
	m_tile_0.Bind(MOTION_BLUR_TILE_0_TEXTURE);
	glBindImageTexture(MOTION_BLUR_TILE_0_IMAGE, m_tile_0.Get(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG16);

	m_tile.Initialize();
	m_tile.Storage(div_ceil(width, tile_size), div_ceil(height, tile_size), GL_RG16, 1);
	m_tile.SetSizeFilter(GL_LINEAR, GL_LINEAR);
	m_tile.SetWrapFilter(GL_CLAMP_TO_EDGE);
	m_tile.Bind(MOTION_BLUR_TILE_TEXTURE);
}
