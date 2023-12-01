#include "Animation.hpp"

#include "Config.hpp"

void Animation::Initialize(const char *obj_file, const char *split_cache_file) {
	{
		auto cornell_mesh = MeshLoader{}.MakeCornellBox({.63f, .065f, .05f}, {.161f, .133f, .427f}, {.725f, .71f, .68f},
		                                                {1.f, 1.f, 1.f});
		m_cornell_gpu_model.Initialize({&cornell_mesh, 1});
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
}

void Animation::Update(float delta_t) {}

void Animation::Draw(int width, int height) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glm::mat4 proj = glm::perspective(glm::pi<float>() / 3.0f, float(width) / float(height), 0.1f, 32.0f);
	glm::mat4 view =
	    glm::lookAt(glm::vec3{.0f, .0f, 1.f + glm::sqrt(3.f)}, glm::vec3{.0f, .0f, .0f}, glm::vec3{.0f, 1.f, .0f});
	m_camera_buffer.Update(proj * view);
	m_camera_buffer.BindUniform(0);

	glViewport(0, 0, width, height);
	{ // G-Buffer
		m_gbuffer.UseFBO(width, height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_mesh_shader.Use();
		// draw model
		m_cornell_gpu_model.Draw();
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