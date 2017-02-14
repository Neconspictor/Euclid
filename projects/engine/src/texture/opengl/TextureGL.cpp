#include <texture/opengl/TextureGL.hpp>
#include <memory>

using namespace std;

TextureGL::TextureGL(): textureID(GL_FALSE)
{
}

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

void TextureGL::release()
{
	glDeleteTextures(1, &textureID);
	textureID = GL_FALSE;
}

void TextureGL::setTexture(GLuint id)
{
	textureID = id;
}

RenderTargetGL::RenderTargetGL() : frameBuffer(GL_FALSE), renderBuffer(GL_FALSE)
{
}

RenderTargetGL::~RenderTargetGL()
{
}

GLuint RenderTargetGL::getFrameBuffer()
{
	return frameBuffer;
}

GLuint RenderTargetGL::getRenderBuffer()
{
	return renderBuffer;
}

GLuint RenderTargetGL::getTextureGL()
{
	return textureBuffer.getTexture();
}

Texture* RenderTargetGL::getTexture()
{
	return &textureBuffer;
}

void RenderTargetGL::setFrameBuffer(GLuint newValue)
{
	frameBuffer = newValue;
}

void RenderTargetGL::setRenderBuffer(GLuint newValue)
{
	renderBuffer = newValue;
}

void RenderTargetGL::setTextureBuffer(GLuint newValue)
{
	textureBuffer.setTexture(newValue);
}

DepthMapGL::DepthMapGL(int width, int height) : DepthMap(width, height)
{
	GLuint textureID = GL_FALSE;
	glGenFramebuffers(1, &frameBuffer);
	glGenTextures(1, &textureID);
	texture.setTexture(textureID);

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

DepthMapGL::DepthMapGL(const DepthMapGL& other) : DepthMap(other),
	texture(other.texture), frameBuffer(other.frameBuffer)
{
}

DepthMapGL::DepthMapGL(DepthMapGL&& other) : DepthMap(other),
    texture(other.texture), frameBuffer(other.frameBuffer)
{
	other.frameBuffer = GL_FALSE;
}

DepthMapGL& DepthMapGL::operator=(const DepthMapGL& other)
{
	if (this == &other)
		return *this;
	// call base asignment operator
	DepthMap::operator =(other);

	texture = move(other.texture);
	frameBuffer = other.frameBuffer;
	return *this;
}

DepthMapGL& DepthMapGL::operator=(DepthMapGL&& other)
{
	if (this == &other)
		return *this;
	// call base asignment operator
	DepthMap::operator =(other);
	texture = move(other.texture);
	frameBuffer = move(other.frameBuffer);
	other.frameBuffer = GL_FALSE;
	return *this;
}

DepthMapGL::~DepthMapGL()
{
}

GLuint DepthMapGL::getFramebuffer() const
{
	return frameBuffer;
}

GLuint DepthMapGL::getTexture() const
{
	return texture.getTexture();
}

Texture* DepthMapGL::getTexture()
{
	return &texture;
}

void DepthMapGL::release()
{
	texture.release();
	glDeleteFramebuffers(1, &frameBuffer);
	frameBuffer = GL_FALSE;
}