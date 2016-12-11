#include <texture/opengl/TextureGL.hpp>
#include <memory>

using namespace std;

TextureGL::TextureGL(GLuint texture) : Texture(), textureID(texture)
{
}

TextureGL::TextureGL(const TextureGL& other)
{
	textureID = other.textureID;
}

TextureGL::TextureGL(TextureGL&& other)
{
	textureID = move(other.textureID);
}

TextureGL& TextureGL::operator=(const TextureGL& other)
{
	if (this == &other) return *this;
	this->textureID = other.textureID;
	return *this;
}

TextureGL& TextureGL::operator=(TextureGL&& other)
{
	if (this == &other) return *this;
	this->textureID = move(other.textureID);
	return *this;
}

TextureGL::~TextureGL()
{
}

GLuint TextureGL::getTexture() const
{
	return textureID;
}

void TextureGL::setTexture(GLuint id)
{
	textureID = id;
}