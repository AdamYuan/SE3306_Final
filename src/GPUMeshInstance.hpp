#pragma once

#include "GPUMesh.hpp"
#include "Transform.hpp"

#include <myvk/CommandBuffer.hpp>

class GPUMeshInstance final : public myvk::DeviceObjectBase {
private:
	struct InstanceInfo {
		glm::vec4 color;
		glm::mat4 model, prev_model;
	};
	myvk::Ptr<GPUMesh> m_mesh_ptr;

	myvk::Ptr<myvk::Buffer> m_instance_buffer;
	InstanceInfo *m_p_instance_info;
	uint32_t m_max_instance_count;

	TransformSet m_prev_set;

public:
	GPUMeshInstance(const myvk::Ptr<GPUMesh> &mesh_ptr, uint32_t max_instance_count);
	inline ~GPUMeshInstance() final = default;

	inline const myvk::Ptr<myvk::Device> &GetDevicePtr() const { return m_mesh_ptr->GetDevicePtr(); }
	void SetTransform(TransformSet &&transforms);
	void CmdDraw(const myvk::Ptr<myvk::CommandBuffer> &command_buffer, int lod);

	inline static std::vector<VkVertexInputBindingDescription> GetVertexInputBindings() {
		return {
		    {.binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX},
		    {.binding = 1, .stride = sizeof(InstanceInfo), .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE},
		};
	}
	inline static std::vector<VkVertexInputAttributeDescription> GetVertexInputAttributes() {
		return {
		    {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, position)},
		    {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, normal)},
		    {.location = 2, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, color)},

		    {.location = 3,
		     .binding = 1,
		     .format = VK_FORMAT_R32G32B32A32_SFLOAT,
		     .offset = offsetof(InstanceInfo, color)},

		    {.location = 4,
		     .binding = 1,
		     .format = VK_FORMAT_R32G32B32A32_SFLOAT,
		     .offset = offsetof(InstanceInfo, model)},
		    {.location = 5,
		     .binding = 1,
		     .format = VK_FORMAT_R32G32B32A32_SFLOAT,
		     .offset = offsetof(InstanceInfo, model) + 4 * sizeof(float)},
		    {.location = 6,
		     .binding = 1,
		     .format = VK_FORMAT_R32G32B32A32_SFLOAT,
		     .offset = offsetof(InstanceInfo, model) + 8 * sizeof(float)},
		    {.location = 7,
		     .binding = 1,
		     .format = VK_FORMAT_R32G32B32A32_SFLOAT,
		     .offset = offsetof(InstanceInfo, model) + 12 * sizeof(float)},

		    {.location = 8,
		     .binding = 1,
		     .format = VK_FORMAT_R32G32B32A32_SFLOAT,
		     .offset = offsetof(InstanceInfo, prev_model)},
		    {.location = 9,
		     .binding = 1,
		     .format = VK_FORMAT_R32G32B32A32_SFLOAT,
		     .offset = offsetof(InstanceInfo, prev_model) + 4 * sizeof(float)},
		    {.location = 10,
		     .binding = 1,
		     .format = VK_FORMAT_R32G32B32A32_SFLOAT,
		     .offset = offsetof(InstanceInfo, prev_model) + 8 * sizeof(float)},
		    {.location = 11,
		     .binding = 1,
		     .format = VK_FORMAT_R32G32B32A32_SFLOAT,
		     .offset = offsetof(InstanceInfo, prev_model) + 12 * sizeof(float)},
		};
	}
};
