#ifndef ENGINE_TEXTURE_OPENGL_TEXTUREGL_HPP
#define ENGINE_TEXTURE_OPENGL_TEXTUREGL_HPP
#include <texture/Texture.hpp>
#include <GL/glew.h>

class TextureGL : public Texture
{
public:
	TextureGL(GLuint texture);
	TextureGL(const TextureGL& other);
	TextureGL(TextureGL&& other);
	TextureGL& operator=(const TextureGL& other);
	TextureGL& operator=(TextureGL&& other);

	virtual ~TextureGL();

	GLuint getTexture() const;

	void setTexture(GLuint id);

private:
	GLuint textureID;
};
#endif