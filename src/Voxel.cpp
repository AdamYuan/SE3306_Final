#include "Voxel.hpp"

#include <shader/Binding.h>

void Voxel::Initialize() {
	{
		m_voxelize_shader.Initialize();
		constexpr const GLuint kVertSPIRV[] = {
#include <shader/voxelize.vert.u32>
		};
		constexpr const GLuint kGeomSPIRV[] = {
#include <shader/voxelize.geom.u32>
		};
		constexpr const GLuint kFragSPIRV[] = {
#include <shader/voxelize.frag.u32>
		};
		m_voxelize_shader.LoadBinary(kVertSPIRV, sizeof(kVertSPIRV), GL_VERTEX_SHADER);
		m_voxelize_shader.LoadBinary(kGeomSPIRV, sizeof(kGeomSPIRV), GL_GEOMETRY_SHADER);
		m_voxelize_shader.LoadBinary(kFragSPIRV, sizeof(kFragSPIRV), GL_FRAGMENT_SHADER);
		m_voxelize_shader.Finalize();
	}
}

void Voxel::initialize_target(int resolution) {
	if (m_resolution == resolution)
		return;
	m_resolution = resolution;

	m_rbo.Initialize();
	m_rbo.Storage(GL_R8, resolution, resolution);

	m_albedo.Initialize();
	m_albedo.Storage(resolution, resolution, resolution, GL_RGBA8, 1);
	m_albedo.SetSizeFilter(GL_NEAREST, GL_NEAREST);
	m_albedo.SetWrapFilter(GL_CLAMP_TO_BORDER);

	m_normal.Initialize();
	m_normal.Storage(resolution, resolution, resolution, GL_RG8_SNORM, 1);
	m_normal.SetSizeFilter(GL_NEAREST, GL_NEAREST);
	m_normal.SetWrapFilter(GL_CLAMP_TO_BORDER);

	m_mipmaps = mygl3::Texture3D::GetLevelCount(resolution, resolution, resolution);

	m_radiance.Initialize();
	m_radiance.Storage(resolution, resolution, resolution, GL_RGBA16F, m_mipmaps);
	m_radiance.SetSizeFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	m_radiance.SetWrapFilter(GL_CLAMP_TO_BORDER);

	glBindImageTexture(VOXEL_ALBEDO_IMAGE, m_albedo.Get(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
	glBindImageTexture(VOXEL_NORMAL_IMAGE, m_normal.Get(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG8_SNORM);
	glBindImageTexture(VOXEL_RADIANCE_IMAGE, m_radiance.Get(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);

	m_albedo.Bind(VOXEL_ALBEDO_TEXTURE);
	m_normal.Bind(VOXEL_NORMAL_TEXTURE);
	m_radiance.Bind(VOXEL_RADIANCE_TEXTURE);
}
