#pragma once

#include "MeshLoader.hpp"

#include "CameraBuffer.hpp"
#include "GBuffer.hpp"
#include "GPUMesh.hpp"
#include "ShadowMap.hpp"
#include "Tumbler.hpp"
#include "Voxel.hpp"

#include <mygl3/shader.hpp>
#include <optional>

class Animation {
private:
	// render objects
	GPUMesh m_cornell_gpu_model, m_tumbler_gpu_model;
	mygl3::Shader m_final_shader;
	mygl3::VertexArray m_quad_vao;
	ShadowMap m_shadow_map;
	GBuffer m_gbuffer;
	CameraBuffer m_camera_buffer;
	Voxel m_voxel;

	Tumbler m_tumbler;

	// drag & drop
	std::optional<glm::vec2> m_drag_pos;

public:
	void Initialize(const char *obj_file);
	void Update(float delta_t, const std::optional<glm::vec2> &drag);
	void Draw(int width, int height);
};
