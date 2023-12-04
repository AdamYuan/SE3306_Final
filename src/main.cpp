#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Animation.hpp"

constexpr int kWidth = 720, kHeight = 720;

Animation animation = {};

static void key_callback(GLFWwindow *, int key, int, int action, int) {
	// if (action == GLFW_PRESS && key == GLFW_KEY_SPACE)
	// animation.Start();
}

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	GLFWwindow *window = glfwCreateWindow(kWidth, kHeight, "Tumbler", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	gl3wInit();

	animation.Initialize("bunny.obj");

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

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
		animation.Update(cur_time - prev_time, drag);

		animation.Draw(kWidth, kHeight);
		glfwSwapBuffers(window);

		prev_time = cur_time;
	}

	return 0;
}
