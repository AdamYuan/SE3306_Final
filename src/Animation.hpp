#pragma once

#include "MeshLoader.hpp"

#include "CameraBuffer.hpp"
#include "GBuffer.hpp"
#include "GPUMesh.hpp"
#include "ShadowMap.hpp"
#include "Playground.hpp"
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

	Playground m_playground;

	// drag & drop
	struct DragInfo {
		bool locked, below_center;
		glm::vec2 xz;
		float plane_y;
	};
	std::optional<DragInfo> m_opt_drag;

public:
	void Initialize(const char *obj_file);
	void Drag(const std::optional<glm::vec2> &opt_drag_pos);
	void Update(float delta_t);
	void Draw(int width, int height);
};
