#pragma once

#include "MeshLoader.hpp"

#include "CameraBuffer.hpp"
#include "GBuffer.hpp"
#include "GPUMesh.hpp"
#include "ShadowMap.hpp"
#include "Voxel.hpp"

#include <mygl3/shader.hpp>

class Animation {
private:
	GPUMesh m_cornell_gpu_model, m_tumbler_gpu_model;

	mygl3::Shader m_final_shader;
	mygl3::VertexArray m_quad_vao;
	ShadowMap m_shadow_map;
	GBuffer m_gbuffer;
	CameraBuffer m_camera_buffer;
	Voxel m_voxel;

public:
	void Initialize(const char *obj_file);
	void Update(float delta_t);
	void Draw(int width, int height);
};
