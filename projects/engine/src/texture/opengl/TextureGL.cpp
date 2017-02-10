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

DepthMapGL::DepthMapGL(int width, int height) : DepthMap(width, height)
{
	glGenFramebuffers(1, &frameBuffer);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);

	// A depth map only needs depth (z-value) informations; therefore disable any color buffers
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

DepthMapGL::~DepthMapGL()
{
	glDeleteTextures(1, &textureID);
	glDeleteFramebuffers(1, &frameBuffer);
}

GLuint DepthMapGL::getFramebuffer() const
{
	return frameBuffer;
}

GLuint DepthMapGL::getTexture() const
{
	return textureID;
}