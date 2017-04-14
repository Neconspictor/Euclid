#include <texture/opengl/TextureGL.hpp>
#include <memory>
#include <cassert>

using namespace std;

CubeMapGL::CubeMapGL() : TextureGL() {}

CubeMapGL::CubeMapGL(GLuint cubeMap) : TextureGL(cubeMap)
{}

CubeMapGL::CubeMapGL(const CubeMapGL& other) : TextureGL(other)
{}

CubeMapGL::CubeMapGL(CubeMapGL&& other) : TextureGL(other)
{}

CubeMapGL& CubeMapGL::operator=(const CubeMapGL& other)
{
	if (this == &other) return *this;
	TextureGL::operator=(other);
	return *this;
}

CubeMapGL& CubeMapGL::operator=(CubeMapGL&& other)
{
	if (this == &other) return *this;
	TextureGL::operator=(other);
	return *this;
}

CubeMapGL::~CubeMapGL()
{
}

GLuint CubeMapGL::getCubeMap() const
{
	return textureID;
}

void CubeMapGL::setCubeMap(GLuint id)
{
	textureID = id;
}

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

void RenderTargetGL::copyFrom(RenderTargetGL* dest, const Dimension& sourceDim, const Dimension& destDim)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, dest->getFrameBuffer());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
	glBlitFramebuffer(sourceDim.xPos, sourceDim.yPos, sourceDim.width, sourceDim.height,
		destDim.xPos, destDim.yPos, destDim.width, destDim.height,
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
		GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

RenderTargetGL RenderTargetGL::createMultisampled(GLint textureChannel, int width, int height, 
	GLuint samples, GLuint depthStencilType)
{
	assert(samples > 1);

	RenderTargetGL result;
	glGenFramebuffers(1, &result.frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, result.frameBuffer);

	// Generate texture
	glGenTextures(1, &result.textureBuffer.textureID);
	const GLuint& textureID = result.textureBuffer.textureID;

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureID);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, textureChannel, width, height, GL_TRUE);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	// attach texture to currently bound frame buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureID, 0);

	//create a render buffer for depth and stencil testing
	glGenRenderbuffers(1, &result.renderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, result.renderBuffer);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, depthStencilType, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// attach render buffer to the frame buffer
	if (depthStencilType == GL_DEPTH_COMPONENT)
	{
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, result.renderBuffer);
	}
	else
	{
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, result.renderBuffer);
	}

	// finally check if all went successfully
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw runtime_error("RendererOpenGL::createRenderTarget(): Couldn't successfully init framebuffer!");
	}

	return result;
}

RenderTargetGL RenderTargetGL::createSingleSampled(GLint textureChannel, int width, int height, GLuint depthStencilType)
{
	RenderTargetGL result;
	glGenFramebuffers(1, &result.frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, result.frameBuffer);

	// Generate texture
	glGenTextures(1, &result.textureBuffer.textureID);
	const GLuint& textureID = result.textureBuffer.textureID;

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, textureChannel, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// clamp is important so that no pixel artifacts occur on the border!
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	// attach texture to currently bound frame buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

	//create a render buffer for depth and stencil testing
	glGenRenderbuffers(1, &result.renderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, result.renderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, depthStencilType, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// attach render buffer to the frame buffer
	if (depthStencilType == GL_DEPTH_COMPONENT)
	{
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, result.renderBuffer);
	}
	else
	{
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, result.renderBuffer);
	}

	// finally check if all went successfully
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw runtime_error("RendererOpenGL::createRenderTarget(): Couldn't successfully init framebuffer!");
	}

	return result;
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

void RenderTargetGL::release()
{
	textureBuffer.release();
	glDeleteFramebuffers(1, &frameBuffer);
	glDeleteRenderbuffers(1, &renderBuffer);

	frameBuffer = GL_FALSE;
	renderBuffer = GL_FALSE;
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

CubeDepthMapGL::CubeDepthMapGL(int width, int height) : CubeDepthMap(width, height), 
frameBuffer(GL_FALSE)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glGenFramebuffers(1, &frameBuffer);
	cubeMap.setCubeMap(texture);
	textureID = texture;

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	for (int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_COMPONENT, GL_RENDERBUFFER, texture);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw runtime_error("CubeDepthMapGL::CubeDepthMapGL(int, int): Framebuffer not complete!");

	// A depth map only needs depth (z-value) informations; therefore disable any color buffers
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

CubeDepthMapGL::CubeDepthMapGL(const CubeDepthMapGL& other) : TextureGL(other), CubeDepthMap(other), cubeMap(other.cubeMap),
frameBuffer(other.frameBuffer)
{
}

CubeDepthMapGL::CubeDepthMapGL(CubeDepthMapGL&& other) : TextureGL(other), CubeDepthMap(other), cubeMap(other.cubeMap),
frameBuffer(other.frameBuffer)
{
	other.frameBuffer = GL_FALSE;
}

CubeDepthMapGL& CubeDepthMapGL::operator=(const CubeDepthMapGL& other)
{
	if (this == &other) return *this;
	CubeDepthMap::operator=(other);
	TextureGL::operator=(other);
	this->frameBuffer = other.frameBuffer;
	this->cubeMap = other.cubeMap;
	return *this;
}

CubeDepthMapGL& CubeDepthMapGL::operator=(CubeDepthMapGL&& other)
{
	if (this == &other) return *this;
	CubeDepthMap::operator=(other);
	TextureGL::operator=(other);
	this->frameBuffer = other.frameBuffer;
	this->cubeMap = other.cubeMap;
	other.frameBuffer = GL_FALSE;
	return *this;
}

CubeDepthMapGL::~CubeDepthMapGL()
{}

GLuint CubeDepthMapGL::getCubeMapTexture() const
{
	return cubeMap.getCubeMap();
}

CubeMap* CubeDepthMapGL::getCubeMap()
{
	return &cubeMap;
}

GLuint CubeDepthMapGL::getFramebuffer() const
{
	return frameBuffer;
}

void CubeDepthMapGL::release()
{
}

DepthMapGL::DepthMapGL(int width, int height) : DepthMap(width, height)
{
	GLuint textureID = GL_FALSE;
	glGenFramebuffers(1, &frameBuffer);
	glGenTextures(1, &textureID);
	texture.setTexture(textureID);
	//GL_RG32F
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	
	/*glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);*/


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

VarianceShadowMapGL::VarianceShadowMapGL(int width, int height): VarianceShadowMap(width, height)
{
	GLuint textureID = GL_FALSE;
	glGenFramebuffers(1, &frameBuffer);
	glGenTextures(1, &textureID);
	texture.setTexture(textureID);
	//GL_RG32F
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0f, 1.0f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	//glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureID, 0);
	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

								   // Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw runtime_error("VarianceShadowMapGL::VarianceShadowMapGL(): Couldn't configure frame buffer!");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

VarianceShadowMapGL::VarianceShadowMapGL(const VarianceShadowMapGL& other) : VarianceShadowMap(other),
texture(other.texture), frameBuffer(other.frameBuffer)
{
}

VarianceShadowMapGL::VarianceShadowMapGL(VarianceShadowMapGL&& other) : VarianceShadowMap(other),
texture(other.texture), frameBuffer(other.frameBuffer)
{
	other.frameBuffer = GL_FALSE;
}

VarianceShadowMapGL& VarianceShadowMapGL::operator=(const VarianceShadowMapGL& other)
{
	if (this == &other)
		return *this;
	// call base asignment operator
	VarianceShadowMap::operator =(other);

	texture = move(other.texture);
	frameBuffer = other.frameBuffer;
	return *this;
}

VarianceShadowMapGL& VarianceShadowMapGL::operator=(VarianceShadowMapGL&& other)
{
	if (this == &other)
		return *this;
	// call base asignment operator
	VarianceShadowMap::operator =(other);
	texture = move(other.texture);
	frameBuffer = move(other.frameBuffer);
	other.frameBuffer = GL_FALSE;
	return *this;
}

VarianceShadowMapGL::~VarianceShadowMapGL(){}

GLuint VarianceShadowMapGL::getFramebuffer() const
{
	return frameBuffer;
}

GLuint VarianceShadowMapGL::getTexture() const
{
	return texture.getTexture();
}

Texture* VarianceShadowMapGL::getTexture()
{
	return &texture;
}

void VarianceShadowMapGL::release()
{
	texture.release();
	glDeleteFramebuffers(1, &frameBuffer);
	frameBuffer = GL_FALSE;
}