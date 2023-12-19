#pragma once

#include "MeshLoader.hpp"

#include "Bloom.hpp"
#include "CameraBuffer.hpp"
#include "GBuffer.hpp"
#include "GPUMesh.hpp"
#include "ParticleSystem.hpp"
#include "Playground.hpp"
#include "ShadowMap.hpp"
#include "TAALight.hpp"
#include "Voxel.hpp"

#include <mygl3/shader.hpp>
#include <mygl3/texture.hpp>
#include <optional>

class Animation {
private:
	// render resources
	GPUMesh m_cornell_gpu_mesh, m_tumbler_gpu_mesh, m_marble_gpu_mesh, m_fireball_gpu_mesh, m_particle_gpu_mesh;
	mygl3::Texture2D m_tumbler_texture, m_floor_texture;
	mygl3::VertexArray m_quad_vao;

	// passes
	mygl3::Shader m_final_shader;
	ShadowMap m_shadow_map;
	GBuffer m_gbuffer;
	CameraBuffer m_camera_buffer;
	Voxel m_voxel;
	Bloom m_bloom;
	TAALight m_taa_light;

	// playground
	Playground m_playground;

	// particles
	ParticleSystem m_particle_system;

	// drag & drop
	struct DragInfo {
		Tumbler *p_tumbler;
		bool below_center;
		glm::vec2 xz;
		float plane_y;
	};
	std::optional<DragInfo> m_opt_drag;
	void drag(const std::optional<glm::vec2> &opt_drag_pos);

	// control flags
	bool m_marbles_flag, m_fire_ball_flag;

public:
	void Initialize();
	void ToggleMarbles();
	void ToggleFireball();
	void Update(float delta_t, const std::optional<glm::vec2> &opt_drag_pos);
	void Draw(int width, int height);
};
