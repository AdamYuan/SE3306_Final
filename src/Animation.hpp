#pragma once

#include "MeshLoader.hpp"

#include "CameraBuffer.hpp"
#include "GBuffer.hpp"
#include "GPUMesh.hpp"
#include "SSAOBuffer.hpp"

#include <mygl3/shader.hpp>
#include <random>

class Animation {
private:
	GPUMesh m_cornell_gpu_model, m_tumbler_gpu_model;

	mygl3::Shader m_mesh_shader, m_ssao_shader, m_final_shader;
	mygl3::VertexArray m_quad_vao;
	GBuffer m_gbuffer;
	SSAOBuffer m_ssao_buffer;
	CameraBuffer m_camera_buffer;

public:
	void Initialize(const char *obj_file, const char *split_cache_file);
	void Update(float delta_t);
	void Draw(int width, int height);
};
