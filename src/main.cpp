#include "Animation.hpp"
#include "GPUAnimation.hpp"
#include "rg/ARenderGraph.hpp"

#include <myvk/FrameManager.hpp>
#include <myvk/GLFWHelper.hpp>
#include <myvk/Instance.hpp>
#include <myvk/Queue.hpp>

constexpr int kWidth = 720, kHeight = 720, kFrameCount = 3;

Animation animation = {};

bool enable_motion_blur = true;

static void key_callback(GLFWwindow *, int key, int, int action, int) {
	if (action != GLFW_PRESS)
		return;

	if (key == GLFW_KEY_S)
		animation.ToggleMarbles();
	else if (key == GLFW_KEY_F)
		animation.ToggleFireball();
	else if (key == GLFW_KEY_M)
		enable_motion_blur = !enable_motion_blur;
}

int main() {
	GLFWwindow *window = myvk::GLFWCreateWindow("Tumbler", kWidth, kHeight, false);
	glfwSetKeyCallback(window, key_callback);

	myvk::Ptr<myvk::Device> device;
	myvk::Ptr<myvk::Queue> generic_queue;
	myvk::Ptr<myvk::PresentQueue> present_queue;
	{
		auto instance = myvk::Instance::CreateWithGlfwExtensions();
		auto surface = myvk::Surface::Create(instance, window);
		auto physical_device = myvk::PhysicalDevice::Fetch(instance)[0];
		auto features = physical_device->GetDefaultFeatures();
		features.vk13.maintenance4 = VK_TRUE;
		// features.vk13.subgroupSizeControl = VK_TRUE;
		// features.vk13.computeFullSubgroups = VK_TRUE;
		device = myvk::Device::Create(physical_device,
		                              myvk::GenericPresentQueueSelector{&generic_queue, surface, &present_queue},
		                              features, {VK_KHR_SWAPCHAIN_EXTENSION_NAME});
	}
	auto frame_manager = myvk::FrameManager::Create(generic_queue, present_queue, true, kFrameCount,
	                                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

	auto command_pool = myvk::CommandPool::Create(generic_queue);

	GPUAMesh gpu_ani_mesh = GPUAMesh::Create(command_pool);
	GPUATexture gpu_ani_texture = GPUATexture::Create(command_pool);

	myvk::Ptr<rg::ARenderGraph> render_graphs[kFrameCount];
	for (uint32_t f = 0; f < kFrameCount; ++f) {
		auto &render_graph = render_graphs[f];
		render_graph =
		    myvk::MakePtr<rg::ARenderGraph>(frame_manager, GPUAInstance::Create(gpu_ani_mesh, gpu_ani_texture), f);
		render_graph->SetCanvasSize(VkExtent2D{kWidth, kHeight});
	}

	auto prev_time = (float)glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		std::optional<glm::vec2> drag = std::nullopt;
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			glm::dvec2 cursor_pos;
			glfwGetCursorPos(window, &cursor_pos.x, &cursor_pos.y);
			drag = glm::vec2{(float)cursor_pos.x / (float)kWidth, (float)cursor_pos.y / (float)kHeight};
		}
		auto cur_time = (float)glfwGetTime();
		float delta_t = std::min(cur_time - prev_time, .1f);
		animation.Update(delta_t, drag);
		prev_time = cur_time;

		if (frame_manager->NewFrame()) {
			uint32_t image_index = frame_manager->GetCurrentImageIndex();
			uint32_t current_frame = frame_manager->GetCurrentFrame();
			const auto &command_buffer = frame_manager->GetCurrentCommandBuffer();
			const auto &render_graph = render_graphs[current_frame];

			render_graph->SetMotionBlur(enable_motion_blur);
			render_graph->Update(animation);

			command_buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			render_graph->CmdExecute(command_buffer);
			command_buffer->End();

			frame_manager->Render();
		}
	}
	frame_manager->WaitIdle();
	glfwTerminate();

	return 0;
}
