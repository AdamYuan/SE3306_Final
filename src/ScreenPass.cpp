#include "ScreenPass.hpp"

void ScreenPass::Initialize(const char *quad_vert_str) {
	m_shader.Initialize();
	constexpr const char *kFinalFrag =
#include <shader/screen.frag.str>
	    ;
	m_shader.Load(quad_vert_str, GL_VERTEX_SHADER);
	m_shader.Load(kFinalFrag, GL_FRAGMENT_SHADER);
	m_shader.Finalize();
}
