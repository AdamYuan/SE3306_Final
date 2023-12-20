#pragma once

#include <mygl3/texture.hpp>

class Texture {
private:
	mygl3::Texture2D m_tumbler_texture, m_floor_texture;

public:
	void Initialize();
};
