#pragma once

#include "Bloom.hpp"
#include "CameraBuffer.hpp"
#include "GBuffer.hpp"
#include "GPUMesh.hpp"
#include "LightPass.hpp"
#include "MeshLoader.hpp"
#include "MotionBlur.hpp"
#include "ParticleSystem.hpp"
#include "Playground.hpp"
#include "ScreenPass.hpp"
#include "ShadowMap.hpp"
#include "TemporalAA.hpp"
#include "Texture.hpp"
#include "Voxel.hpp"

#include <mygl3/shader.hpp>
#include <mygl3/texture.hpp>
#include <optional>

class Animation {
private:
	// render resources
	GPUMesh m_cornell_gpu_mesh, m_tumbler_gpu_mesh, m_marble_gpu_mesh, m_fireball_gpu_mesh, m_particle_gpu_mesh;
	mygl3::VertexArray m_quad_vao;
	Texture m_texture;

	// passes
	ShadowMap m_shadow_map;
	GBuffer m_gbuffer;
	CameraBuffer m_camera_buffer;
	Voxel m_voxel;
	Bloom m_bloom;
	MotionBlur m_motion_blur;
	LightPass m_light_pass;
	TemporalAA m_taa;
	ScreenPass m_screen_pass;

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
	void drag(float delta_t, const std::optional<glm::vec2> &opt_drag_pos);

	// control flags
	bool m_marbles_flag = false, m_fire_ball_flag = false, m_motion_blur_flag = true;

public:
	void Initialize();
	void ToggleMarbles();
	void ToggleFireball();
	inline void ToggleMotionBlur() { m_motion_blur_flag ^= 1; }
	void Update(float delta_t, const std::optional<glm::vec2> &opt_drag_pos);
	void Draw(float delta_t, int width, int height);
};
