#include <sprite/opengl/SpriteGL.hpp>

SpriteGL::SpriteGL(int width, int height, const glm::vec2& position, TextureGL* texture): 
	Sprite(width, height, position), texture(texture)
{}

SpriteGL::~SpriteGL()
{
}

Texture* SpriteGL::getTexture() const
{
	return texture;
}

void SpriteGL::draw()
{
	TextureGL* textureGL = static_cast<TextureGL*>(texture);
	GLuint textureID = textureGL->getTexture();
	// TODO draw texture to screen
}