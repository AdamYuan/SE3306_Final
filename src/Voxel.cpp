#include "Voxel.hpp"

#include <shader/Binding.h>

void Voxel::Initialize() {
	{
		m_voxelize_shader.Initialize();
		constexpr const char *kVert = {
#include <shader/voxelize.vert.str>
		};
		constexpr const char *kGeom = {
#include <shader/voxelize.geom.str>
		};
		constexpr const char *kFrag = {
#include <shader/voxelize.frag.str>
		};
		m_voxelize_shader.Load(kVert, GL_VERTEX_SHADER);
		m_voxelize_shader.Load(kGeom, GL_GEOMETRY_SHADER);
		m_voxelize_shader.Load(kFrag, GL_FRAGMENT_SHADER);
		m_voxelize_shader.Finalize();
	}
	{
		m_mipmap_0_shader.Initialize();
		constexpr const char *kComp =
#include <shader/voxel_mipmap_0.comp.str>
		    ;
		m_mipmap_0_shader.Load(kComp, GL_COMPUTE_SHADER);
		m_mipmap_0_shader.Finalize();
	}
	{
		m_mipmap_shader.Initialize();
		constexpr const char *kComp =
#include <shader/voxel_mipmap.comp.str>
		    ;
		m_mipmap_shader.Load(kComp, GL_COMPUTE_SHADER);
		m_mipmap_shader.Finalize();
	}
}

void Voxel::generate_mipmap() {
	const auto dispatch = [](int x, int y, int z) {
		glDispatchCompute(x / 4 + (x % 4 != 0), y / 4 + (y % 4 != 0), z / 4 + (z % 4 != 0));
	};

	for (int i = 0; i < 6; ++i)
		glBindImageTexture(VOXEL_RADIANCE_MIPMAP_IMAGE + i, m_radiance_mipmaps[i].Get(), 0, GL_TRUE, 0, GL_WRITE_ONLY,
		                   GL_RGBA16F);
	m_mipmap_0_shader.Use();
	dispatch(m_resolution >> 1, m_resolution >> 1, m_resolution >> 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	for (int l = 2; l < m_mipmaps; ++l) {
		for (int i = 0; i < 6; ++i)
			glBindImageTexture(VOXEL_RADIANCE_MIPMAP_IMAGE + i, m_radiance_mipmaps[i].Get(), l - 1, GL_TRUE, 0,
			                   GL_WRITE_ONLY, GL_RGBA16F);
		m_mipmap_shader.Use();
		m_mipmap_shader.SetInt(0, l - 2);
		dispatch(m_resolution >> l, m_resolution >> l, 6 * (m_resolution >> l));
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
}

void Voxel::initialize_target(int resolution, int mipmaps) {
	if (m_resolution == resolution && m_mipmaps == mipmaps)
		return;
	m_resolution = resolution;
	m_mipmaps = mipmaps;

	m_rbo.Initialize();
	m_rbo.Storage(GL_R8, resolution, resolution);

	/* m_albedo.Initialize();
	m_albedo.Storage(resolution, resolution, resolution, GL_RGBA8, 1);
	m_albedo.SetSizeFilter(GL_NEAREST, GL_NEAREST);
	m_albedo.SetWrapFilter(GL_CLAMP_TO_BORDER);

	m_normal.Initialize();
	m_normal.Storage(resolution, resolution, resolution, GL_RG8_SNORM, 1);
	m_normal.SetSizeFilter(GL_NEAREST, GL_NEAREST);
	m_normal.SetWrapFilter(GL_CLAMP_TO_BORDER); */

	m_radiance.Initialize();
	m_radiance.Storage(resolution, resolution, resolution, GL_RGBA16F, 1);
	m_radiance.SetSizeFilter(GL_LINEAR, GL_LINEAR);
	m_radiance.SetWrapFilter(GL_CLAMP_TO_BORDER);

	for (auto &m : m_radiance_mipmaps) {
		m.Initialize();
		m.Storage(resolution / 2, resolution / 2, resolution / 2, GL_RGBA16F, mipmaps - 1);
		m.SetSizeFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		m.SetWrapFilter(GL_CLAMP_TO_BORDER);
	}

	// glBindImageTexture(VOXEL_ALBEDO_IMAGE, m_albedo.Get(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
	// glBindImageTexture(VOXEL_NORMAL_IMAGE, m_normal.Get(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG8_SNORM);
	glBindImageTexture(VOXEL_RADIANCE_IMAGE, m_radiance.Get(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);

	// m_albedo.Bind(VOXEL_ALBEDO_TEXTURE);
	// m_normal.Bind(VOXEL_NORMAL_TEXTURE);
	m_radiance.Bind(VOXEL_RADIANCE_TEXTURE);
	for (int i = 0; i < 6; ++i)
		m_radiance_mipmaps[i].Bind(VOXEL_RADIANCE_MIPMAP_TEXTURE + i);
}
