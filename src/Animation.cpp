#include "Animation.hpp"

#include "Config.hpp"

constexpr int kShadowMapSize = 720;

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
	{
		m_mesh_shader.Initialize();
		constexpr const GLuint kMeshVertSPIRV[] = {
#include <shader/mesh.vert.u32>
		};
		constexpr const GLuint kMeshFragSPIRV[] = {
#include <shader/mesh.frag.u32>
		};
		m_mesh_shader.LoadBinary(kMeshVertSPIRV, sizeof(kMeshVertSPIRV), GL_VERTEX_SHADER);
		m_mesh_shader.LoadBinary(kMeshFragSPIRV, sizeof(kMeshFragSPIRV), GL_FRAGMENT_SHADER);
		m_mesh_shader.Finalize();
	}

	{
		m_shadow_shader.Initialize();
		constexpr const GLuint kShadowVertSPIRV[] = {
#include <shader/shadow.vert.u32>
		};
		constexpr const GLuint kShadowFragSPIRV[] = {
#include <shader/shadow.frag.u32>
		};
		m_shadow_shader.LoadBinary(kShadowVertSPIRV, sizeof(kShadowVertSPIRV), GL_VERTEX_SHADER);
		m_shadow_shader.LoadBinary(kShadowFragSPIRV, sizeof(kShadowFragSPIRV), GL_FRAGMENT_SHADER);
		m_shadow_shader.Finalize();
	}

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
	{
		m_ssao_shader.Initialize();
		constexpr const GLuint kSSAOFragSPIRV[] = {
#include <shader/ssao.frag.u32>
		};
		m_ssao_shader.LoadBinary(kQuadVertSPIRV, sizeof(kQuadVertSPIRV), GL_VERTEX_SHADER);
		m_ssao_shader.LoadBinary(kSSAOFragSPIRV, sizeof(kSSAOFragSPIRV), GL_FRAGMENT_SHADER);
		m_ssao_shader.Finalize();
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
}

void Animation::Update(float delta_t) {}

void Animation::Draw(int width, int height) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	m_camera_buffer.BindUniform(0);

	// shadow map view
	glViewport(0, 0, kShadowMapSize, kShadowMapSize);
	glCullFace(GL_FRONT);
	{
		m_shadow_map.UseFBO(kShadowMapSize, kShadowMapSize);
		glClear(GL_DEPTH_BUFFER_BIT);
		m_shadow_shader.Use();
		m_tumbler_gpu_model.Draw();
	}

	m_shadow_map.BindOutput(6);

	// main view
	glViewport(0, 0, width, height);
	glCullFace(GL_BACK);
	{ // G-Buffer
		m_gbuffer.UseFBO(width, height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_mesh_shader.Use();
		// draw model
		m_cornell_gpu_model.Draw();
		m_tumbler_gpu_model.Draw();
	}

	// Post process
	glDisable(GL_DEPTH_TEST);
	m_quad_vao.Bind();

	m_gbuffer.BindAlbedoOutput(0);
	m_gbuffer.BindNormalOutput(1);
	m_gbuffer.BindDepthOutput(2);

	{ // SSAO
		m_ssao_buffer.UseFBO(width, height);

		m_ssao_buffer.BindSampleTexture(4);
		m_ssao_buffer.BindNoiseTexture(5);

		m_ssao_shader.Use();
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	{ // Final pass
		mygl3::FrameBuffer::Unbind();

		m_ssao_buffer.BindOcclusionOutput(3);

		m_final_shader.Use();
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
}