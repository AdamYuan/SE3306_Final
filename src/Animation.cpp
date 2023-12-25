#include "Animation.hpp"

#include "MeshLoader.hpp"
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
static const glm::mat4 kShadowViewProj =
    glm::perspective(gcem::atan(1.f / (kCornellLightHeight - 1.f)) * 2.f, 1.f, Z_NEAR, Z_FAR) *
    glm::lookAt(glm::vec3{.0f, kCornellLightHeight, .0f}, glm::vec3{.0f, .0f, .0f}, glm::vec3{.0f, .0f, 1.f});

Mesh Animation::GetCornellMesh(int lod) {
	const auto make_cornell = [](uint32_t ico_subdivision) {
		return MeshLoader{}.MakeCornellBox(kCornellLeftColor, kCornellRightColor, kCornelFloorTextureID,
		                                   kCornellOtherColor, kCornellLightRadiance, kCornellLightHeight,
		                                   kCornellLightRadius, ico_subdivision);
	};
	return lod ? make_cornell(2) : make_cornell(4);
}
Mesh Animation::GetTumblerMesh(int lod) {
	return lod ? MeshLoader{}.MakeTumbler(3, 16, kTumblerTextureID)
	           : MeshLoader{}.MakeTumbler(10, 100, kTumblerTextureID);
}
Mesh Animation::GetMarbleMesh(int lod) { return MeshLoader{}.MakeUVSphere(Marble::kRadius, 20, kCornelFloorTextureID); }
Mesh Animation::GetFireballMesh(int lod) {
	if (lod) {
		Mesh solid_lod_fireball = MeshLoader{}.MakeIcoSphere(Fireball::kRadius, 1, kFireballRadiance);
		solid_lod_fireball.Combine(MeshLoader{}.MakeIcoSphere(Fireball::kRadius * 0.9f, 0, kFireballRadiance));
		solid_lod_fireball.Combine(MeshLoader{}.MakeIcoSphere(Fireball::kRadius * 0.8f, 0, kFireballRadiance));
		solid_lod_fireball.Combine(MeshLoader{}.MakeIcoSphere(Fireball::kRadius * 0.7f, 0, kFireballRadiance));
		solid_lod_fireball.Combine(MeshLoader{}.MakeIcoSphere(Fireball::kRadius * 0.5f, 0, kFireballRadiance));
		solid_lod_fireball.Combine(MeshLoader{}.MakeIcoSphere(Fireball::kRadius * 0.3f, 0, kFireballRadiance));
		return solid_lod_fireball;
	}
	return MeshLoader{}.MakeIcoSphere(Fireball::kRadius, 4, kFireballRadiance);
}
Mesh Animation::GetParticleMesh(int lod) {
	return lod ? MeshLoader{}.MakeIcoSphere(1.f, 0, {}) : MeshLoader{}.MakeIcoSphere(1.f, 2, {});
}

uint32_t Animation::GetTumblerCount() { return kTumblerCount; }
uint32_t Animation::GetMaxMarbleCount() { return kMarbleCount; }
uint32_t Animation::GetMaxParticleCount() { return kMaxParticleCount; }

glm::mat4 Animation::GetCameraViewProj() { return kCameraViewProj; }
glm::mat4 Animation::GetInvCameraViewProj() { return kInvCameraViewProj; }
glm::mat4 Animation::GetShadowViewProj() { return kShadowViewProj; }

Animation::Animation() {
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
	m_particle_system.Update(delta_t);
	m_playground.Update(
	    delta_t, [this, delta_t, &opt_drag_pos]() { drag(delta_t, opt_drag_pos); },
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
