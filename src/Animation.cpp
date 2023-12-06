#include "Animation.hpp"

#include <gcem.hpp>
#include <shader/Config.h>

constexpr int kShadowMapSize = 480, kVoxelResolution = 64, kVoxelMipmaps = 5;

constexpr glm::vec3 kCornellLeftColor = {.953f, .357f, .212f}, kCornellRightColor = {.486f, .631f, .663},
                    kCornellOtherColor = {.725f, .71f, .68f};
constexpr glm::vec3 kTumblerColor = {.63f, .065f, .05f};

constexpr uint32_t kTumblerCount = 5;
constexpr float kTumblerPlaceRadius = 0.6f;

constexpr float kCameraFov = glm::pi<float>() / 3.f;
constexpr glm::vec3 kCameraPos = {.0f, .0f, 1.f + 1.f / gcem::tan(kCameraFov * 0.5f)};

static const glm::mat4 kCameraViewProj = glm::perspective(kCameraFov, 1.f, Z_NEAR, Z_FAR) *
                                         glm::lookAt(kCameraPos, glm::vec3{.0f, .0f, .0f}, glm::vec3{.0f, 1.f, .0f});
static const glm::mat4 kInvCameraViewProj = glm::inverse(kCameraViewProj);

void Animation::Initialize(const char *obj_file) {
	{
		auto cornell_mesh =
		    MeshLoader{}.MakeCornellBox(kCornellLeftColor, kCornellRightColor, kCornellOtherColor,
		                                kCornellLightRadiance, kCornellLightHeight, kCornellLightRadius);
		m_cornell_gpu_model.Initialize({&cornell_mesh, 1});
	}
	{
		// auto model = MeshLoader{}.Load(obj_file, kCornellOtherColor);
		auto tumbler_mesh = MeshLoader{}.MakeTumbler(10, 100, kTumblerColor);
		m_tumbler_gpu_model.Initialize({&tumbler_mesh, 1}, {&kTumblerCount, 1});
		// m_tumbler_gpu_model.Initialize(std::vector<Mesh>{std::move(tumbler_mesh), std::move(model)});
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
	glm::mat4 shadow_proj = glm::perspective(glm::atan(1.f / (kCornellLightHeight - 1.f)) * 2.f, 1.f, Z_NEAR, Z_FAR);
	glm::mat4 shadow_view =
	    glm::lookAt(glm::vec3{.0f, kCornellLightHeight, .0f}, glm::vec3{.0f, .0f, .0f}, glm::vec3{.0f, .0f, 1.f});
	m_camera_buffer.Update(kCameraViewProj, kInvCameraViewProj, shadow_proj * shadow_view);

	m_gbuffer.Initialize();
	m_shadow_map.Initialize();
	m_voxel.Initialize();

	m_playground.Initialize(kTumblerCount, kTumblerPlaceRadius);
}

#include <glm/gtx/string_cast.hpp>
void Animation::Update(float delta_t, const std::optional<glm::vec2> &opt_drag_pos) {
	m_playground.Update(delta_t);

	m_playground.SetTumblerMesh(&m_tumbler_gpu_model);

	// process drag
	if (opt_drag_pos.has_value()) {
		glm::vec3 dir;
		{
			glm::vec4 clip = glm::vec4{opt_drag_pos.value() * 2.0f - 1.0f, 1.0f, 1.0f};
			clip.y = -clip.y;
			glm::vec4 world = kInvCameraViewProj * clip;
			world /= world.w;
			dir = glm::normalize(glm::vec3{world} - kCameraPos);
		}
		if (m_opt_drag && m_opt_drag.value().locked) {
			// already dragging
			auto &drag = m_opt_drag.value();
			if (drag.below_center) {
				float t = (drag.plane_y - kCameraPos.y) / dir.y;
				if (t > 1e-4f) {
					glm::vec2 pos = (kCameraPos + dir * t).xz();
					pos = glm::clamp(pos, glm::vec2(-2.f), glm::vec2(2.f));
					m_playground.MoveLockedTumbler(pos - drag.xz);
					drag.xz = pos;
				}
			} else {
				glm::vec2 pos = opt_drag_pos.value();
				m_playground.RotateLockedTumbler(pos - drag.xz, 0.015f);
				drag.xz = pos;
			}
		} else if (!m_opt_drag) {
			auto opt_lock = m_playground.TryLockTumbler(kCameraPos, dir);
			if (opt_lock) {
				auto lock = opt_lock.value();
				if (lock.below_center) {
					glm::vec3 pos = kCameraPos + dir * lock.t;
					m_opt_drag = DragInfo{
					    .locked = true,
					    .below_center = true,
					    .xz = glm::vec2{pos.x, pos.z},
					    .plane_y = pos.y,
					};
				} else
					m_opt_drag = DragInfo{.locked = true, .below_center = false, .xz = opt_drag_pos.value()};
			} else
				m_opt_drag = DragInfo{.locked = false};
		}
	} else {
		m_playground.UnlockTumbler();
		m_opt_drag = std::nullopt;
	}
}

void Animation::Draw(int width, int height) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// shadow map
	glViewport(0, 0, kShadowMapSize, kShadowMapSize);
	glCullFace(GL_FRONT);
	m_shadow_map.Generate(kShadowMapSize, kShadowMapSize, [this]() {
		glClear(GL_DEPTH_BUFFER_BIT);
		m_tumbler_gpu_model.Draw();
	});

	// voxels
	glViewport(0, 0, kVoxelResolution, kVoxelResolution);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	m_voxel.Generate(kVoxelResolution, kVoxelMipmaps, [this]() {
		glClear(GL_COLOR_BUFFER_BIT);
		m_cornell_gpu_model.Draw();
		m_tumbler_gpu_model.Draw();
	});

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// main view
	glViewport(0, 0, width, height);
	glCullFace(GL_BACK);
	m_gbuffer.Generate(width, height, [this]() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// draw model
		m_cornell_gpu_model.Draw();
		m_tumbler_gpu_model.Draw();
	});

	// Post process
	glDisable(GL_DEPTH_TEST);
	m_quad_vao.Bind();

	{ // Final pass
		mygl3::FrameBuffer::Unbind();

		m_final_shader.Use();
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
}
