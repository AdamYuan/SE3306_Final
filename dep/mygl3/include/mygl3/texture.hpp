//
// Created by adamyuan on 4/14/18.
//

#ifndef MYGL3_TEXTURE_HPP
#define MYGL3_TEXTURE_HPP

#include "flags.hpp"

namespace mygl3 {
// deleted mipmap
#define DEF_TEXTURE_CLASS(NAME, TARGET) \
	class NAME { \
	private: \
		GLuint id_{kInvalidOglId}; \
\
	public: \
		NAME() = default; \
		NAME &operator=(const NAME &) = delete; \
		NAME(NAME &&texture) noexcept : id_(texture.id_) { texture.id_ = kInvalidOglId; } \
		~NAME() { \
			if (IsValidOglId(id_)) { \
				glDeleteTextures(1, &id_); \
				id_ = kInvalidOglId; \
			} \
		} \
		NAME(const NAME &) = delete; \
		void Initialize() { \
			if (IsValidOglId(id_)) { \
				glDeleteTextures(1, &id_); \
				id_ = kInvalidOglId; \
			} \
			glCreateTextures(TARGET, 1, &id_); \
		} \
		void Bind(GLuint unit) const { glBindTextureUnit(unit, id_); } \
		void SetSizeFilter(GLenum min_filter, GLenum mag_filter) { \
			glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, min_filter); \
			glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, mag_filter); \
		} \
		void SetWrapFilter(GLenum filter) { \
			glTextureParameteri(id_, GL_TEXTURE_WRAP_S, filter); \
			glTextureParameteri(id_, GL_TEXTURE_WRAP_T, filter); \
			glTextureParameteri(id_, GL_TEXTURE_WRAP_R, filter); \
		} \
		GLuint Get() const { return id_; }

DEF_TEXTURE_CLASS(Texture2D, GL_TEXTURE_2D)
static GLsizei GetLevelCount(GLsizei width, GLsizei height) {
	GLsizei cnt = 1;
	while ((width | height) >> cnt)
		++cnt;
	return cnt;
}
void Storage(GLsizei width, GLsizei height, GLenum internal_format, GLsizei levels = 1) {
	glTextureStorage2D(id_, levels, internal_format, width, height);
}
void Data(const GLvoid *pixels, GLsizei width, GLsizei height, GLenum format, GLenum type, GLint level = 0) {
	glTextureSubImage2D(id_, level, 0, 0, width, height, format, type, pixels);
}
void GenerateMipmap() { glGenerateTextureMipmap(id_); }
};

DEF_TEXTURE_CLASS(Texture1D, GL_TEXTURE_1D)
static GLsizei GetLevelCount(GLsizei width) {
	GLsizei cnt = 1;
	while (width >> cnt)
		++cnt;
	return cnt;
}
void Storage(GLsizei width, GLenum internal_format, GLsizei levels = 1) {
	glTextureStorage1D(id_, levels, internal_format, width);
}
void Data(const GLvoid *pixels, GLsizei width, GLenum format, GLenum type, GLint level = 0) {
	glTextureSubImage1D(id_, level, 0, width, format, type, pixels);
}
void GenerateMipmap() { glGenerateTextureMipmap(id_); }
}
;

DEF_TEXTURE_CLASS(Texture2DArray, GL_TEXTURE_2D_ARRAY)
static GLsizei GetLevelCount(GLsizei width, GLsizei height) {
	GLsizei cnt = 1;
	while ((width | height) >> cnt)
		++cnt;
	return cnt;
}
void Storage(GLsizei width, GLsizei height, GLsizei depth, GLenum internal_format, GLsizei levels = 1) {
	glTextureStorage3D(id_, levels, internal_format, width, height, depth);
}
void Data(const GLvoid *pixels, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type,
          GLint level = 0) {
	glTextureSubImage3D(id_, level, 0, 0, 0, width, height, depth, format, type, pixels);
}
void GenerateMipmap() { glGenerateTextureMipmap(id_); }
}
;

DEF_TEXTURE_CLASS(Texture3D, GL_TEXTURE_3D)
static GLsizei GetLevelCount(GLsizei width, GLsizei height, GLsizei depth) {
	GLsizei cnt = 1;
	while ((width | height | depth) >> cnt)
		++cnt;
	return cnt;
}
void Storage(GLsizei width, GLsizei height, GLsizei depth, GLenum internal_format, GLsizei levels = 1) {
	glTextureStorage3D(id_, levels, internal_format, width, height, depth);
}
void Data(const GLvoid *pixels, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type,
          GLint level = 0) {
	glTextureSubImage3D(id_, level, 0, 0, 0, width, height, depth, format, type, pixels);
}
void GenerateMipmap() { glGenerateTextureMipmap(id_); }
}
;
}

#endif // MYGL3_TEXTURE_HPP
