#include "GPUAnimation.hpp"

#include <stb_image.h>

GPUAMesh GPUAMesh::Create(const myvk::Ptr<myvk::CommandPool> &command_pool) {
	GPUAMesh ret = {};
	ret.cornell_mesh = myvk::MakePtr<GPUMesh>(
	    command_pool, std::initializer_list<Mesh>{Animation::GetCornellMesh(0), Animation::GetCornellMesh(1)});
	ret.tumbler_mesh = myvk::MakePtr<GPUMesh>(
	    command_pool, std::initializer_list<Mesh>{Animation::GetTumblerMesh(0), Animation::GetTumblerMesh(1)});
	ret.marble_mesh = myvk::MakePtr<GPUMesh>(
	    command_pool, std::initializer_list<Mesh>{Animation::GetMarbleMesh(0), Animation::GetMarbleMesh(1)});
	ret.fireball_mesh = myvk::MakePtr<GPUMesh>(
	    command_pool, std::initializer_list<Mesh>{Animation::GetFireballMesh(0), Animation::GetFireballMesh(1)});
	ret.particle_mesh = myvk::MakePtr<GPUMesh>(
	    command_pool, std::initializer_list<Mesh>{Animation::GetParticleMesh(0), Animation::GetParticleMesh(1)});
	return ret;
}

GPUATexture GPUATexture::Create(const myvk::Ptr<myvk::CommandPool> &command_pool) {
	const auto &device = command_pool->GetDevicePtr();
	GPUATexture ret = {};

	myvk::Ptr<myvk::Buffer> tumbler_staging, floor_staging;
	myvk::Ptr<myvk::Image> floor_image, tumbler_image;
	{
		constexpr unsigned char kTumblerPNG[] = {
#include <texture/tumbler.png.u8>
		};
		int x, y, c;
		stbi_uc *img = stbi_load_from_memory(kTumblerPNG, sizeof(kTumblerPNG), &x, &y, &c, 4);
		tumbler_staging = myvk::Buffer::CreateStaging(device, img, img + x * y * 4);
		stbi_image_free(img);

		auto extent = VkExtent2D{(uint32_t)x, (uint32_t)y};
		tumbler_image = myvk::Image::CreateTexture2D(
		    device, extent, myvk::Image::QueryMipLevel(extent), VK_FORMAT_R8G8B8A8_UNORM,
		    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		ret.tumbler_image_view = myvk::ImageView::Create(tumbler_image, VK_IMAGE_VIEW_TYPE_2D);
	}
	{
		constexpr unsigned char kFloorJPG[] = {
#include <texture/floor.jpg.u8>
		};
		int x, y, c;
		stbi_uc *img = stbi_load_from_memory(kFloorJPG, sizeof(kFloorJPG), &x, &y, &c, 4);
		floor_staging = myvk::Buffer::CreateStaging(device, img, img + x * y * 4);
		stbi_image_free(img);

		auto extent = VkExtent2D{(uint32_t)x, (uint32_t)y};
		floor_image = myvk::Image::CreateTexture2D(
		    device, extent, myvk::Image::QueryMipLevel(extent), VK_FORMAT_R8G8B8A8_UNORM,
		    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		ret.floor_image_view = myvk::ImageView::Create(floor_image, VK_IMAGE_VIEW_TYPE_2D);
	}

	ret.tumbler_sampler =
	    myvk::Sampler::CreateClampToBorder(device, VK_FILTER_LINEAR, VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK);
	ret.floor_sampler = myvk::Sampler::Create(device, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);

	VkSampler tumbler_immutable_sampler = ret.tumbler_sampler->GetHandle();
	VkSampler floor_immutable_sampler = ret.floor_sampler->GetHandle();

	auto descriptor_pool = myvk::DescriptorPool::Create(
	    device, 1, {{.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 2}});
	auto descriptor_set_layout = myvk::DescriptorSetLayout::Create(
	    device, {VkDescriptorSetLayoutBinding{.binding = 0,
	                                          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	                                          .descriptorCount = 1,
	                                          .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
	                                          .pImmutableSamplers = &tumbler_immutable_sampler},
	             VkDescriptorSetLayoutBinding{.binding = 1,
	                                          .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
	                                          .descriptorCount = 1,
	                                          .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
	                                          .pImmutableSamplers = &floor_immutable_sampler}});

	ret.descriptor_set = myvk::DescriptorSet::Create(descriptor_pool, descriptor_set_layout);
	ret.descriptor_set->UpdateCombinedImageSampler(ret.tumbler_sampler, ret.tumbler_image_view, 0);
	ret.descriptor_set->UpdateCombinedImageSampler(ret.floor_sampler, ret.floor_image_view, 1);

	auto command_buffer = myvk::CommandBuffer::Create(command_pool);
	command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	command_buffer->CmdPipelineBarrier(
	    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, {}, {},
	    {
	        tumbler_image->GetMemoryBarrier(VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
	                                        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
	        floor_image->GetMemoryBarrier(VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
	                                      VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL),
	    });
	command_buffer->CmdCopy(
	    tumbler_staging, tumbler_image,
	    {VkBufferImageCopy{.imageSubresource =
	                           VkImageSubresourceLayers{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1},
	                       .imageOffset = {},
	                       .imageExtent = tumbler_image->GetExtent()}});
	command_buffer->CmdCopy(
	    floor_staging, floor_image,
	    {VkBufferImageCopy{.imageSubresource =
	                           VkImageSubresourceLayers{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .layerCount = 1},
	                       .imageOffset = {},
	                       .imageExtent = floor_image->GetExtent()}});
	command_buffer->CmdGenerateMipmap2D(tumbler_image, VK_PIPELINE_STAGE_TRANSFER_BIT,
	                                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
	                                    VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	command_buffer->CmdGenerateMipmap2D(floor_image, VK_PIPELINE_STAGE_TRANSFER_BIT,
	                                    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
	                                    VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	command_buffer->End();

	auto fence = myvk::Fence::Create(device);
	command_buffer->Submit(fence);
	fence->Wait();

	return ret;
}

GPUAInstance GPUAInstance::Create(const GPUAMesh &gpu_ani_mesh, const GPUATexture &gpu_ani_texture) {
	GPUAInstance ret = {};
	ret.m_cornell_instance = myvk::MakePtr<GPUMeshInstance>(gpu_ani_mesh.cornell_mesh, 1);
	ret.m_tumbler_instance = myvk::MakePtr<GPUMeshInstance>(gpu_ani_mesh.tumbler_mesh, Animation::GetTumblerCount());
	ret.m_marble_instance = myvk::MakePtr<GPUMeshInstance>(gpu_ani_mesh.marble_mesh, Animation::GetMaxMarbleCount());
	ret.m_fireball_instance = myvk::MakePtr<GPUMeshInstance>(gpu_ani_mesh.fireball_mesh, 1);
	ret.m_particle_instance =
	    myvk::MakePtr<GPUMeshInstance>(gpu_ani_mesh.particle_mesh, Animation::GetMaxParticleCount());

	ret.m_gpu_ani_texture = gpu_ani_texture;
	return ret;
}
void GPUAInstance::Update(const Animation &animation) {
	m_cornell_instance->SetTransform({{0, Transform{.model = glm::identity<glm::mat4>(), .color = {}}}});
	m_tumbler_instance->SetTransform(animation.GetTumblerTransforms());
	m_marble_instance->SetTransform(animation.GetMarbleTransforms());
	m_fireball_instance->SetTransform(animation.GetFireballTransforms());
	m_particle_instance->SetTransform(animation.GetParticleTransforms());
}

void GPUAInstance::CmdDraw(const myvk::Ptr<myvk::CommandBuffer> &command_buffer, const ADrawConfig &config) const {
	if (config.opt_cornell_lod)
		m_cornell_instance->CmdDraw(command_buffer, *config.opt_cornell_lod);
	if (config.opt_tumbler_lod)
		m_tumbler_instance->CmdDraw(command_buffer, *config.opt_tumbler_lod);
	if (config.opt_marble_lod)
		m_marble_instance->CmdDraw(command_buffer, *config.opt_marble_lod);
	if (config.opt_fireball_lod)
		m_fireball_instance->CmdDraw(command_buffer, *config.opt_fireball_lod);
	if (config.opt_particle_lod)
		m_particle_instance->CmdDraw(command_buffer, *config.opt_particle_lod);
}
