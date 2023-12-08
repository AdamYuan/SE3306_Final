#include "Animation.hpp"

#include <gcem.hpp>
#include <shader/Binding.h>
#include <shader/Config.h>
#include <stb_image.h>

constexpr int kShadowMapSize = 480, kVoxelResolution = 64, kVoxelMipmaps = 5;

constexpr glm::vec3 kCornellLeftColor = {.953f, .357f, .212f}, kCornellRightColor = {.486f, .631f, .663},
                    kCornellOtherColor = {.725f, .71f, .68f};
constexpr uint32_t kCornelFloorTextureID = 1;

constexpr uint32_t kTumblerCount = 5;
constexpr uint32_t kTumblerTextureID = 2;
constexpr glm::vec3 kTumblerColor = {.63f, .065f, .05f};
constexpr float kTumblerPlaceRadius = 0.6f;

constexpr uint32_t kMarbleCount = 30;
constexpr float kMarbleMinSpeed = 2.f, kMarbleMaxSpeed = 4.f;

constexpr glm::vec3 kFireballRadiance = glm::vec3{1.f, .4588f, .05f} * 5.f;
constexpr float kFireballSpeed = 2.f;

constexpr uint32_t kMaxParticleCount = 4096;

constexpr float kCameraFov = glm::pi<float>() / 3.f;
constexpr glm::vec3 kCameraPos = {.0f, .0f, 1.f + 1.f / gcem::tan(kCameraFov * 0.5f)};

static const glm::mat4 kCameraViewProj = glm::perspective(kCameraFov, 1.f, Z_NEAR, Z_FAR) *
                                         glm::lookAt(kCameraPos, glm::vec3{.0f, .0f, .0f}, glm::vec3{.0f, 1.f, .0f});
static const glm::mat4 kInvCameraViewProj = glm::inverse(kCameraViewProj);

void Animation::Initialize() {
	// Load Meshes
	{
		auto cornell_mesh = MeshLoader{}.MakeCornellBox(kCornellLeftColor, kCornellRightColor, kCornelFloorTextureID,
		                                                kCornellOtherColor, kCornellLightRadiance, kCornellLightHeight,
		                                                kCornellLightRadius);
		m_cornell_gpu_mesh.Initialize({&cornell_mesh, 1});
	}
	{
		auto tumbler_mesh = MeshLoader{}.MakeTumbler(10, 100, kTumblerTextureID);
		m_tumbler_gpu_mesh.Initialize({&tumbler_mesh, 1}, {&kTumblerCount, 1});
	}
	{
		auto marble_mesh = MeshLoader{}.MakeUVSphere(Marble::kRadius, 20, kCornelFloorTextureID);
		m_marble_gpu_mesh.Initialize({&marble_mesh, 1}, {&kMarbleCount, 1});
	}
	{
		auto fireball_mesh = MeshLoader{}.MakeIcoSphere(Fireball::kRadius, 4, kFireballRadiance);
		m_fireball_gpu_mesh.Initialize({&fireball_mesh, 1});
	}
	{
		auto particle_mesh = MeshLoader{}.MakeIcoSphere(1.f, 2, {});
		m_particle_gpu_mesh.Initialize({&particle_mesh, 1}, {&kMaxParticleCount, 1});
	}

	// Load Textures
	{
		constexpr unsigned char kTumblerPNG[] = {
#include <texture/tumbler.png.u8>
		};
		int x, y, c;
		stbi_uc *img = stbi_load_from_memory(kTumblerPNG, sizeof(kTumblerPNG), &x, &y, &c, 4);
		m_tumbler_texture.Initialize();
		m_tumbler_texture.Storage(x, y, GL_RGBA8, mygl3::Texture2D::GetLevelCount(x, y));
		m_tumbler_texture.Data(img, x, y, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		stbi_image_free(img);
		m_tumbler_texture.SetWrapFilter(GL_CLAMP_TO_BORDER);
		m_tumbler_texture.SetSizeFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		m_tumbler_texture.GenerateMipmap();
		m_tumbler_texture.Bind(TUMBLER_TEXTURE);
	}

	// Load Shaders
	constexpr const char *kQuadVert =
#include <shader/quad.vert.str>
	    ;
	{
		m_final_shader.Initialize();
		constexpr const char *kFinalFrag =
#include <shader/final.frag.str>
		    ;
		m_final_shader.Load(kQuadVert, GL_VERTEX_SHADER);
		m_final_shader.Load(kFinalFrag, GL_FRAGMENT_SHADER);
		m_final_shader.Finalize();
	}

	m_quad_vao.Initialize();

	m_camera_buffer.Initialize();
	glm::mat4 shadow_proj = glm::perspective(gcem::atan(1.f / (kCornellLightHeight - 1.f)) * 2.f, 1.f, Z_NEAR, Z_FAR);
	glm::mat4 shadow_view =
	    glm::lookAt(glm::vec3{.0f, kCornellLightHeight, .0f}, glm::vec3{.0f, .0f, .0f}, glm::vec3{.0f, .0f, 1.f});
	m_camera_buffer.Update(kCameraViewProj, kInvCameraViewProj, shadow_proj * shadow_view);

	m_gbuffer.Initialize();
	m_shadow_map.Initialize();
	m_voxel.Initialize();
	m_bloom.Initialize();

	m_playground.Initialize(kTumblerCount, kTumblerPlaceRadius);
	m_particle_system.Initialize(kMaxParticleCount);
}

void Animation::Drag(const std::optional<glm::vec2> &opt_drag_pos) {
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
			drag.p_tumbler->RotateLocked(pos - drag.xz, 0.015f);
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
		m_playground.CreateMarbles(kMarbleCount, glm::vec4{kCornellOtherColor, 1.0f}, kMarbleMinSpeed, kMarbleMaxSpeed);
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

void Animation::Update(float delta_t) {
	m_particle_system.Update(delta_t);
	m_playground.Update(
	    delta_t,
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
		    case SphereHitType::kLight:
			    p_marble->color = {kCornellLightRadiance, 1.0f};
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
	    [this](Fireball *p_fireball, SphereHitInfo hit_info) {
		    if (hit_info.type != SphereHitType::kSphere)
			    m_particle_system.EmitSparks(hit_info.position, hit_info.gradient);
	    },
	    [this, delta_t](const Fireball &fireball) { m_particle_system.SustainFire(fireball, delta_t); });

	m_playground.PopTumblerMesh(&m_tumbler_gpu_mesh);
	m_playground.PopMarbleMesh(&m_marble_gpu_mesh);
	m_playground.PopFireballMesh(&m_fireball_gpu_mesh);
	m_particle_system.PopMesh(&m_particle_gpu_mesh);
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
		m_marble_gpu_mesh.Draw();
	});

	// Voxels
	glViewport(0, 0, kVoxelResolution, kVoxelResolution);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	m_voxel.Generate(kVoxelResolution, kVoxelMipmaps, [this]() {
		glClear(GL_COLOR_BUFFER_BIT);
		m_cornell_gpu_mesh.Draw();
		m_tumbler_gpu_mesh.Draw();
		// m_marble_gpu_mesh.Draw();
		// m_particle_gpu_mesh.Draw();
		m_fireball_gpu_mesh.Draw();
	});

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// G-Buffer
	glViewport(0, 0, width, height);
	glCullFace(GL_BACK);
	m_gbuffer.Generate(width, height, [this]() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		m_cornell_gpu_mesh.Draw();
		m_tumbler_gpu_mesh.Draw();
		m_marble_gpu_mesh.Draw();
		m_fireball_gpu_mesh.Draw();
		m_particle_gpu_mesh.Draw();
	});

	// Post Process
	glDisable(GL_DEPTH_TEST);
	m_quad_vao.Bind();

	m_bloom.Generate(width, height, []() { glDrawArrays(GL_TRIANGLES, 0, 3); });

	{
		mygl3::FrameBuffer::Unbind();

		m_final_shader.Use();
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
}
