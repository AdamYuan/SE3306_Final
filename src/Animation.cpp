#include "Animation.hpp"

#include <gcem.hpp>
#include <shader/Config.h>

constexpr int kShadowMapSize = 480, kVoxelResolution = 64, kVoxelMipmaps = 7;

constexpr glm::vec3 kCornellLeftColor = {.953f, .357f, .212f}, kCornellRightColor = {.486f, .631f, .663},
                    kCornellOtherColor = {.725f, .71f, .68f};
constexpr uint32_t kCornelFloorTextureID = 1;

constexpr uint32_t kTumblerCount = 5;
constexpr uint32_t kTumblerTextureID = 2;
constexpr glm::vec3 kTumblerColor = {.63f, .065f, .05f};
constexpr float kTumblerPlaceRadius = 0.6f;

constexpr uint32_t kMarbleCount = 30;
constexpr float kMarbleMinSpeed = 2.f, kMarbleMaxSpeed = 4.f;

constexpr glm::vec3 kFireballRadiance = glm::vec3{1.f, .4588f, .01f} * 20.f;
constexpr float kFireballSpeed = 2.f;

constexpr uint32_t kMaxParticleCount = 2048;

constexpr float kCameraFov = glm::pi<float>() / 3.f;
constexpr glm::vec3 kCameraPos = {.0f, .0f, 1.f + 1.f / gcem::tan(kCameraFov * 0.5f)};

static const glm::mat4 kCameraViewProj = glm::perspective(kCameraFov, 1.f, Z_NEAR, Z_FAR) *
                                         glm::lookAt(kCameraPos, glm::vec3{.0f, .0f, .0f}, glm::vec3{.0f, 1.f, .0f});
static const glm::mat4 kInvCameraViewProj = glm::inverse(kCameraViewProj);

void Animation::Initialize() {
	// Load Meshes
	{
		const auto make_cornell = [](uint32_t ico_subdivision) {
			return MeshLoader{}.MakeCornellBox(kCornellLeftColor, kCornellRightColor, kCornelFloorTextureID,
			                                   kCornellOtherColor, kCornellLightRadiance, kCornellLightHeight,
			                                   kCornellLightRadius, ico_subdivision);
		};
		m_cornell_gpu_mesh.Initialize(std::initializer_list<Mesh>{make_cornell(4), make_cornell(2)});
		m_tumbler_gpu_mesh.Initialize(std::initializer_list<Mesh>{MeshLoader{}.MakeTumbler(10, 100, kTumblerTextureID),
		                                                          MeshLoader{}.MakeTumbler(3, 16, kTumblerTextureID)},
		                              kTumblerCount);
		m_marble_gpu_mesh.Initialize(
		    std::initializer_list<Mesh>{MeshLoader{}.MakeUVSphere(Marble::kRadius, 20, kCornelFloorTextureID)},
		    kMarbleCount);
		Mesh solid_lod_fireball = MeshLoader{}.MakeIcoSphere(Fireball::kRadius, 1, kFireballRadiance);
		solid_lod_fireball.Combine(MeshLoader{}.MakeIcoSphere(Fireball::kRadius * 0.9f, 0, kFireballRadiance));
		solid_lod_fireball.Combine(MeshLoader{}.MakeIcoSphere(Fireball::kRadius * 0.8f, 0, kFireballRadiance));
		solid_lod_fireball.Combine(MeshLoader{}.MakeIcoSphere(Fireball::kRadius * 0.7f, 0, kFireballRadiance));
		solid_lod_fireball.Combine(MeshLoader{}.MakeIcoSphere(Fireball::kRadius * 0.5f, 0, kFireballRadiance));
		solid_lod_fireball.Combine(MeshLoader{}.MakeIcoSphere(Fireball::kRadius * 0.3f, 0, kFireballRadiance));
		m_fireball_gpu_mesh.Initialize(std::initializer_list<Mesh>{
		    MeshLoader{}.MakeIcoSphere(Fireball::kRadius, 4, kFireballRadiance), std::move(solid_lod_fireball)});
		m_particle_gpu_mesh.Initialize(
		    std::initializer_list<Mesh>{MeshLoader{}.MakeIcoSphere(1.f, 2, {}), MeshLoader{}.MakeIcoSphere(1.f, 0, {})},
		    kMaxParticleCount);
	}

	// Load Shaders
	m_quad_vao.Initialize();

	m_camera_buffer.Initialize();
	glm::mat4 shadow_proj = glm::perspective(gcem::atan(1.f / (kCornellLightHeight - 1.f)) * 2.f, 1.f, Z_NEAR, Z_FAR);
	glm::mat4 shadow_view =
	    glm::lookAt(glm::vec3{.0f, kCornellLightHeight, .0f}, glm::vec3{.0f, .0f, .0f}, glm::vec3{.0f, .0f, 1.f});
	m_camera_buffer.Update(kCameraViewProj, kInvCameraViewProj, shadow_proj * shadow_view);

	m_texture.Initialize();
	m_gbuffer.Initialize();
	m_shadow_map.Initialize();
	m_voxel.Initialize();
	constexpr const char *kQuadVert =
#include <shader/quad.vert.str>
	    ;
	m_bloom.Initialize(kQuadVert);
	m_motion_blur.Initialize(kQuadVert);
	m_light_pass.Initialize(kQuadVert);
	m_taa.Initialize(kQuadVert);
	m_screen_pass.Initialize(kQuadVert);

	m_playground.Initialize(kTumblerCount, kTumblerPlaceRadius);
	m_particle_system.Initialize(kMaxParticleCount);
}

void Animation::drag(float delta_t, const std::optional<glm::vec2> &opt_drag_pos) {
	if (!opt_drag_pos) {
		if (m_opt_drag && m_opt_drag.value().p_tumbler)
			m_opt_drag.value().p_tumbler->locked = false;
		m_opt_drag = std::nullopt;
		return;
	}
	// process drag
	glm::vec3 dir;
	{
		glm::vec4 clip = glm::vec4{opt_drag_pos.value() * 2.0f - 1.0f, 1.0f, 1.0f};
		clip.y = -clip.y;
		glm::vec4 world = kInvCameraViewProj * clip;
		world /= world.w;
		dir = glm::normalize(glm::vec3{world} - kCameraPos);
	}

	if (m_opt_drag && m_opt_drag.value().p_tumbler) {
		// already dragging
		auto &drag = m_opt_drag.value();
		if (drag.below_center) {
			float t = (drag.plane_y - kCameraPos.y) / dir.y;
			if (t > 1e-4f) {
				glm::vec2 pos = (kCameraPos + dir * t).xz();
				pos = glm::clamp(pos, glm::vec2(-2.f), glm::vec2(2.f));
				drag.p_tumbler->MoveLocked(pos - drag.xz);
				drag.xz = pos;
			}
		} else {
			glm::vec2 pos = opt_drag_pos.value();
			drag.p_tumbler->RotateLocked(pos - drag.xz, 3.5f * delta_t);
			drag.xz = pos;
		}
	} else if (!m_opt_drag) {
		auto opt_cast = m_playground.RayCastTumbler(kCameraPos, dir);
		if (opt_cast) {
			auto cast = opt_cast.value();
			glm::vec3 pos = kCameraPos + dir * cast.t;

			cast.p_tumbler->locked = true;
			if (pos.y <= -1.f + Tumbler::kBottomRadius) {
				m_opt_drag = DragInfo{
				    .p_tumbler = cast.p_tumbler,
				    .below_center = true,
				    .xz = glm::vec2{pos.x, pos.z},
				    .plane_y = pos.y,
				};
			} else
				m_opt_drag = DragInfo{.p_tumbler = cast.p_tumbler, .below_center = false, .xz = opt_drag_pos.value()};
		} else
			m_opt_drag = DragInfo{.p_tumbler = nullptr};
	}
}

void Animation::ToggleMarbles() {
	if (!m_marbles_flag)
		m_playground.CreateMarbles(kMarbleCount, glm::vec4{1.0f}, kMarbleMinSpeed, kMarbleMaxSpeed);
	else
		m_playground.DeleteMarbles();

	m_marbles_flag ^= 1;
}

void Animation::ToggleFireball() {
	if (!m_fire_ball_flag)
		m_playground.CreateFireball(kFireballSpeed);
	else
		m_playground.DeleteFireball();
	m_fire_ball_flag ^= 1;
}

void Animation::Update(float delta_t, const std::optional<glm::vec2> &opt_drag_pos) {
	m_particle_system.Update(delta_t, &m_particle_gpu_mesh);
	m_playground.Update(
	    delta_t, &m_tumbler_gpu_mesh, &m_marble_gpu_mesh, &m_fireball_gpu_mesh,
	    [this, delta_t, &opt_drag_pos]() { drag(delta_t, opt_drag_pos); },
	    [this](Marble *p_marble, SphereHitInfo info) {
		    switch (info.type) {
		    case SphereHitType::kFront:
			    return;
		    case SphereHitType::kLeft:
			    p_marble->color = {kCornellLeftColor, 1.0f};
			    return;
		    case SphereHitType::kRight:
			    p_marble->color = {kCornellRightColor, 1.0f};
			    return;
		    case SphereHitType::kBottom:
			    p_marble->color = {};
			    return;
		    case SphereHitType::kTumbler:
			    p_marble->color = {kTumblerColor, 1.0f};
			    return;
		    case SphereHitType::kSphere:
			    m_particle_system.EmitAshes(*p_marble);
			    p_marble->alive = false;
			    return;
		    default:
			    p_marble->color = {kCornellOtherColor, 1.0f};
			    return;
		    }
	    },
	    [this]() { m_marbles_flag = false; },
	    [this](Fireball *p_fireball, SphereHitInfo hit_info) {
		    if (hit_info.type != SphereHitType::kSphere)
			    m_particle_system.EmitSparks(hit_info.position, hit_info.gradient);
	    },
	    [this, delta_t](const Fireball &fireball) { m_particle_system.SustainFire(fireball, delta_t); });
}

void Animation::Draw(int width, int height) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Shadow Map
	glViewport(0, 0, kShadowMapSize, kShadowMapSize);
	glCullFace(GL_FRONT);
	m_shadow_map.Generate(kShadowMapSize, kShadowMapSize, [this]() {
		glClear(GL_DEPTH_BUFFER_BIT);
		m_tumbler_gpu_mesh.Draw();
	});

	// Voxels
	glViewport(0, 0, kVoxelResolution, kVoxelResolution);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	m_voxel.Generate(kVoxelResolution, kVoxelMipmaps, [this]() {
		glClear(GL_COLOR_BUFFER_BIT);
		m_cornell_gpu_mesh.Draw(1);
		m_tumbler_gpu_mesh.Draw(1);
		m_particle_gpu_mesh.Draw(1);
		m_fireball_gpu_mesh.Draw(1);
	});

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Draw Marbles to Shadow Map
	glViewport(0, 0, kShadowMapSize, kShadowMapSize);
	glCullFace(GL_FRONT);
	m_shadow_map.Generate(kShadowMapSize, kShadowMapSize, [this]() { m_marble_gpu_mesh.Draw(); });

	glm::vec2 jitter = m_taa.GetJitter(width, height);

	// G-Buffer
	glViewport(0, 0, width, height);
	glCullFace(GL_BACK);
	m_gbuffer.Generate(width, height, jitter, [this]() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_cornell_gpu_mesh.Draw();
		m_tumbler_gpu_mesh.Draw();
		m_marble_gpu_mesh.Draw();
		m_fireball_gpu_mesh.Draw();
		m_particle_gpu_mesh.Draw();
	});

	// Quad
	glDisable(GL_DEPTH_TEST);
	m_quad_vao.Bind();

	// Motion Blur (Velocity Tile)
	m_motion_blur.GenerateVelocityTile(width, height, 20, []() { glDrawArrays(GL_TRIANGLES, 0, 3); });

	// Generate Bloom
	m_bloom.Generate(width, height, 5, 0.005f, [](int w, int h) {
		glViewport(0, 0, w, h);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	});

	glViewport(0, 0, width, height);

	// Light Pass
	m_light_pass.Generate(width, height, []() { glDrawArrays(GL_TRIANGLES, 0, 3); });
	// TAA
	m_taa.Generate(width, height, jitter, []() { glDrawArrays(GL_TRIANGLES, 0, 3); });

	// Screen Pass
	m_screen_pass.Generate(jitter, []() { glDrawArrays(GL_TRIANGLES, 0, 3); });
}
