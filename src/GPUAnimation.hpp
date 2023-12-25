#pragma once

#include "Animation.hpp"
#include "GPUMesh.hpp"
#include "GPUMeshInstance.hpp"

#include <myvk/CommandBuffer.hpp>
#include <myvk/DescriptorSet.hpp>
#include <myvk/Image.hpp>

struct GPUAMesh {
	myvk::Ptr<GPUMesh> cornell_mesh, tumbler_mesh, marble_mesh, fireball_mesh, particle_mesh;
	static GPUAMesh Create(const myvk::Ptr<myvk::CommandPool> &command_pool);
};

struct GPUATexture {
	myvk::Ptr<myvk::ImageView> floor_image_view, tumbler_image_view;
	myvk::Ptr<myvk::Sampler> floor_sampler, tumbler_sampler;
	myvk::Ptr<myvk::DescriptorSet> descriptor_set;
	static GPUATexture Create(const myvk::Ptr<myvk::CommandPool> &command_pool);
};

struct ADrawConfig {
	std::optional<int> opt_cornell_lod, opt_tumbler_lod, opt_marble_lod, opt_fireball_lod, opt_particle_lod;
};

struct GPUAInstance {
private:
	myvk::Ptr<GPUMeshInstance> m_cornell_instance, m_tumbler_instance, m_marble_instance, m_fireball_instance,
	    m_particle_instance;
	GPUATexture m_gpu_ani_texture;

public:
	static GPUAInstance Create(const GPUAMesh &gpu_ani_mesh, const GPUATexture &gpu_ani_texture);
	void Update(const Animation &animation);
	inline const auto &GetDescriptorSet() const { return m_gpu_ani_texture.descriptor_set; }
	void CmdDraw(const myvk::Ptr<myvk::CommandBuffer> &command_buffer, const ADrawConfig &config) const;
};
