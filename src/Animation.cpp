#include "Animation.hpp"

#include "Config.hpp"
#include <gcem.hpp>
#include <shader/Config.h>

constexpr int kShadowMapSize = 480, kVoxelResolution = 64, kVoxelMipmaps = 5;

constexpr glm::vec3 kCornellLeftColor = {.953f, .357f, .212f}, kCornellRightColor = {.486f, .631f, .663},
                    kCornellOtherColor = {.725f, .71f, .68f};
constexpr glm::vec3 kTumblerColor = {.63f, .065f, .05f};

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
		m_tumbler_gpu_model.Initialize({&tumbler_mesh, 1});
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
}

#include <glm/gtx/string_cast.hpp>
void Animation::Update(float delta_t, const std::optional<glm::vec2> &drag) {
	static float angle{};
	angle += delta_t;

	m_tumbler.center = glm::vec3(glm::cos(angle), -1.0f + Tumbler::kBottomRadius, glm::sin(angle));
	m_tumbler.angular_velocity.x = 0.1;
	m_tumbler.Update(delta_t);

	m_tumbler_gpu_model.SetModel(0, m_tumbler.GetModel());

	if (drag.has_value()) {
		glm::vec3 dir;
		{
			glm::vec4 clip = glm::vec4{drag.value() * 2.0f - 1.0f, 1.0f, 1.0f};
			clip.y = -clip.y;
			glm::vec4 world = kInvCameraViewProj * clip;
			world /= world.w;
			dir = glm::normalize(glm::vec3{world} - kCameraPos);
		}
		auto opt_t = m_tumbler.RayCast(kCameraPos, dir);
		if (opt_t.has_value())
			printf("%f\n", opt_t.value());
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
