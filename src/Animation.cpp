#include "Animation.hpp"

#include "Config.hpp"

constexpr int kShadowMapSize = 480;

constexpr float kZNear = .1f, kZFar = 4.f;
constexpr float kCornellLightHeight = 1.5f, kCornellLightRadius = 0.6f;
constexpr glm::vec3 kCornellLeftColor = {.63f, .065f, .05f}, kCornellRightColor = {.161f, .133f, .427f},
                    kCornellOtherColor = {.725f, .71f, .68f}, kCornellLightColor = {1.f, 1.f, 1.f};

void Animation::Initialize(const char *obj_file) {
	{
		auto cornell_mesh = MeshLoader{}.MakeCornellBox(kCornellLeftColor, kCornellRightColor, kCornellOtherColor,
		                                                kCornellLightColor, kCornellLightHeight, kCornellLightRadius);
		m_cornell_gpu_model.Initialize({&cornell_mesh, 1});
	}
	{
		// auto tumbler_mesh = MeshLoader{}.Load(obj_file, kCornellOtherColor);
		auto tumbler_mesh = MeshLoader{}.MakeSphere(1.0f, 4, glm::vec3{.0f, 1.f, .0f});
		tumbler_mesh.Normalize(true);
		m_tumbler_gpu_model.Initialize({&tumbler_mesh, 1});

		{
			auto trans = glm::identity<glm::mat4>();
			float scale = .3f;
			trans[0][0] = scale;
			trans[1][1] = scale;
			trans[2][2] = scale;
			trans[3] = glm::vec4(glm::vec3(.2f, -1.f, .4f), 1.f);
			m_tumbler_gpu_model.SetModel(0, trans);
		}
	}

	// Load Shaders
	constexpr const GLuint kQuadVertSPIRV[] = {
#include <shader/quad.vert.u32>
	};
	{
		m_final_shader.Initialize();
		constexpr const GLuint kFinalFragSPIRV[] = {
#include <shader/final.frag.u32>
		};
		m_final_shader.LoadBinary(kQuadVertSPIRV, sizeof(kQuadVertSPIRV), GL_VERTEX_SHADER);
		m_final_shader.LoadBinary(kFinalFragSPIRV, sizeof(kFinalFragSPIRV), GL_FRAGMENT_SHADER);
		m_final_shader.Finalize();
	}

	m_quad_vao.Initialize();

	m_camera_buffer.Initialize();
	glm::mat4 proj = glm::perspective(glm::pi<float>() / 3.0f, 1.f, kZNear, kZFar); // aspect ratio = 1
	glm::mat4 view =
	    glm::lookAt(glm::vec3{.0f, .0f, 1.f + glm::sqrt(3.f)}, glm::vec3{.0f, .0f, .0f}, glm::vec3{.0f, 1.f, .0f});

	glm::mat4 shadow_proj = glm::perspective(glm::atan(1.f / (kCornellLightHeight - 1.f)) * 2.f, 1.f, kZNear, kZFar);
	glm::mat4 shadow_view =
	    glm::lookAt(glm::vec3{.0f, kCornellLightHeight, .0f}, glm::vec3{.0f, .0f, .0f}, glm::vec3{.0f, .0f, 1.f});
	m_camera_buffer.Update(proj * view, shadow_proj * shadow_view);

	m_gbuffer.Initialize();
	m_shadow_map.Initialize();
}

void Animation::Update(float delta_t) {}

void Animation::Draw(int width, int height) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// shadow map view
	glViewport(0, 0, kShadowMapSize, kShadowMapSize);
	glCullFace(GL_FRONT);
	m_shadow_map.Generate(kShadowMapSize, kShadowMapSize, [this]() {
		glClear(GL_DEPTH_BUFFER_BIT);
		m_tumbler_gpu_model.Draw();
	});

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