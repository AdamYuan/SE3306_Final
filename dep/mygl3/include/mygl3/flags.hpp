//
// Created by adamyuan on 12/8/18.
//

#ifndef MYGL3_FLAGS_HPP
#define MYGL3_FLAGS_HPP

#include <GL/gl3w.h>
#include <set>
#include <span>
#include <string>
#include <string_view>

namespace mygl3 {
constexpr GLuint kInvalidOglId = 0xffffffffu;
static bool IsValidOglId(GLuint id) { return id != kInvalidOglId; }
inline static void SyncGPU() {
	GLsync sync_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	GLenum wait_return = GL_UNSIGNALED;
	while (wait_return != GL_ALREADY_SIGNALED && wait_return != GL_CONDITION_SATISFIED)
		wait_return = glClientWaitSync(sync_fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
	glDeleteSync(sync_fence);
}
inline static bool IsExtensionSupported(std::string_view ext) {
	GLint ext_num = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &ext_num);
	for (int i = 0; i < ext_num; ++i)
		if (ext == (const char *)glGetStringi(GL_EXTENSIONS, i))
			return true;
	return false;
}
inline static bool IsExtensionSupported(std::span<const std::string_view> exts) {
	std::set<std::string> ext_set(exts.begin(), exts.end());
	GLint ext_num = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &ext_num);
	for (int i = 0; i < ext_num; ++i)
		ext_set.erase((const char *)glGetStringi(GL_EXTENSIONS, i));
	return ext_set.empty();
}
} // namespace mygl3

#endif
