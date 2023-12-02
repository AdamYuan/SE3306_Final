#pragma once

#include <mygl3/framebuffer.hpp>
#include <mygl3/shader.hpp>
#include <mygl3/texture.hpp>

class Voxel {
private:
	mygl3::Shader m_voxelize_shader;

	mygl3::Texture3D m_albedo, m_normal, m_radiance;
	mygl3::RenderBuffer m_rbo;
	int m_resolution{-1}, m_mipmaps{};

	void initialize_target(int resolution);

public:
	void Initialize();
	template <typename DrawFunc> void Generate(int resolution, DrawFunc &&draw_func) {
		initialize_target(resolution);
		glClearTexSubImage(m_radiance.Get(), 0, 0, 0, 0, resolution, resolution, resolution, GL_RGBA, GL_FLOAT,
		                   nullptr);
		m_rbo.Bind();
		m_voxelize_shader.Use();
		draw_func();
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		m_radiance.GenerateMipmap();
	}
};
