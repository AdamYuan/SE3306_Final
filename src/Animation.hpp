#pragma once

#include "Mesh.hpp"
#include "ParticleSystem.hpp"
#include "Playground.hpp"
#include "Transform.hpp"

#include <optional>

class Animation {
private:
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
	bool m_marbles_flag = false, m_fire_ball_flag = false;

public:
	Animation();
	void ToggleMarbles();
	void ToggleFireball();
	void Update(float delta_t, const std::optional<glm::vec2> &opt_drag_pos);

	inline auto GetTumblerTransforms() const { return m_playground.GetTumblerTransforms(); }
	inline auto GetMarbleTransforms() const { return m_playground.GetMarbleTransforms(); }
	inline auto GetFireballTransforms() const { return m_playground.GetFireballTransforms(); }
	inline auto GetParticleTransforms() const { return m_particle_system.GetTransforms(); }

	static Mesh GetCornellMesh(int lod);
	static Mesh GetTumblerMesh(int lod);
	static Mesh GetMarbleMesh(int lod);
	static Mesh GetFireballMesh(int lod);
	static Mesh GetParticleMesh(int lod);

	static uint32_t GetTumblerCount();
	static uint32_t GetMaxMarbleCount();
	static uint32_t GetMaxParticleCount();

	static glm::mat4 GetCameraViewProj();
	static glm::mat4 GetInvCameraViewProj();
	static glm::mat4 GetShadowViewProj();
};
