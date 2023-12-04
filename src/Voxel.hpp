#pragma once

#include <mygl3/framebuffer.hpp>
#include <mygl3/shader.hpp>
#include <mygl3/texture.hpp>

class Voxel {
private:
	mygl3::Shader m_voxelize_shader, m_mipmap_0_shader, m_mipmap_shader;

	mygl3::Texture3D /*m_albedo, m_normal, */ m_radiance, m_radiance_mipmaps[6];
	mygl3::RenderBuffer m_rbo;
	int m_resolution{-1}, m_mipmaps{-1};

	void initialize_target(int resolution, int mipmaps);
	void generate_mipmap();

public:
	void Initialize();
	// resolution must be power of 2
	template <typename DrawFunc> void Generate(int resolution, int mipmaps, DrawFunc &&draw_func) {
		initialize_target(resolution, mipmaps);
		glClearTexSubImage(m_radiance.Get(), 0, 0, 0, 0, resolution, resolution, resolution, GL_RGBA, GL_FLOAT,
		                   nullptr);
		m_rbo.Bind();
		m_voxelize_shader.Use();
		draw_func();
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		generate_mipmap();
	}
};
