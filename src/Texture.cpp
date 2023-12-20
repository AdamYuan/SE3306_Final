#include "Texture.hpp"

#include <shader/Binding.h>
#include <stb_image.h>

void Texture::Initialize() {
	{
		constexpr unsigned char kTumblerPNG[] = {
#include <texture/tumbler.png.u8>
		};
		int x, y, c;
		stbi_uc *img = stbi_load_from_memory(kTumblerPNG, sizeof(kTumblerPNG), &x, &y, &c, 4);
		m_tumbler_texture.Initialize();
		m_tumbler_texture.Storage(x, y, GL_RGBA8, mygl3::Texture2D::GetLevelCount(x, y));
		m_tumbler_texture.Data(img, x, y, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		stbi_image_free(img);
		m_tumbler_texture.SetWrapFilter(GL_CLAMP_TO_BORDER);
		m_tumbler_texture.SetSizeFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		m_tumbler_texture.GenerateMipmap();
		m_tumbler_texture.Bind(TUMBLER_TEXTURE);
	}
	{
		constexpr unsigned char kFloorJPG[] = {
#include <texture/floor.jpg.u8>
		};
		int x, y, c;
		stbi_uc *img = stbi_load_from_memory(kFloorJPG, sizeof(kFloorJPG), &x, &y, &c, 4);
		m_floor_texture.Initialize();
		m_floor_texture.Storage(x, y, GL_RGBA8, mygl3::Texture2D::GetLevelCount(x, y));
		m_floor_texture.Data(img, x, y, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		stbi_image_free(img);
		m_floor_texture.SetWrapFilter(GL_REPEAT);
		m_floor_texture.SetSizeFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		m_floor_texture.GenerateMipmap();
		m_floor_texture.Bind(FLOOR_TEXTURE);
	}

}
