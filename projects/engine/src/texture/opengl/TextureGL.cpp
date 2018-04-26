#include <texture/opengl/TextureGL.hpp>
#include <memory>
#include <cassert>
#include <renderer/opengl/RendererOpenGL.hpp>
#include <texture/opengl/TextureManagerGL.hpp>

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

GLuint CubeMapGL::mapCubeSideToSystemAxis(Side side)
{
	switch (side) {
	case POSITIVE_X:
		return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
	case NEGATIVE_X:
		return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
	case POSITIVE_Y:
		return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
	case NEGATIVE_Y:
		return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
	case POSITIVE_Z:
		return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
	case NEGATIVE_Z:
		return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
	default:
		throw std::runtime_error("No mapping defined for " + side);
	}
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

CubeRenderTargetGL::CubeRenderTargetGL(int width, int height) : CubeRenderTarget(width, height), frameBuffer(GL_FALSE), renderBuffer(GL_FALSE)
{
	// generate framebuffer and renderbuffer with a depth component
	glGenFramebuffers(1, &frameBuffer);
	glGenRenderbuffers(1, &renderBuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);


	//pre-allocate the six faces of the cubemap
	glGenTextures(1, &cubeMapResult.textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapResult.textureID);
	for (int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

CubeRenderTargetGL::~CubeRenderTargetGL()
{
}

CubeMap * CubeRenderTargetGL::createCopy()
{

	//first create a new cube render target that we use to blit the content
	CubeRenderTargetGL copy(width, height);

	for (int i = 0; i < 6; ++i) {

		//attach the cubemap side of this render target
		glBindFramebuffer(GL_FRAMEBUFFER,  frameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubeMapResult.textureID, 0);

		//attach the cubemap side of the copy render target
		glBindFramebuffer(GL_FRAMEBUFFER, copy.frameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, copy.cubeMapResult.textureID, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// now we can blit the content to the copy
		glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, copy.frameBuffer);
		glBlitFramebuffer(0, 0, width, height,
			0, 0, copy.width, copy.height,
			GL_COLOR_BUFFER_BIT,
			GL_NEAREST);
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	//extract cubemap texture from the copy and delete copy
	unsigned int cache = copy.cubeMapResult.textureID;
	glBindFramebuffer(GL_FRAMEBUFFER, copy.frameBuffer);
	for (int i = 0; i < 6; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0); // unbound the cubemap side
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// reset the texture id of the cubemap of the copy, so that it won't be released!
	copy.cubeMapResult.textureID = GL_FALSE;

	copy.release();

	//register the cubeMap to the texture manager and return the result
	CubeMapGL result(cache);
	return TextureManagerGL::get()->addCubeMap(move(result));
}

GLuint CubeRenderTargetGL::getFrameBuffer()
{
	return frameBuffer;
}

GLuint CubeRenderTargetGL::getRenderBuffer()
{
	return renderBuffer;
}

GLuint CubeRenderTargetGL::getCubeMapGL()
{
	return cubeMapResult.getCubeMap();
}

CubeMap * CubeRenderTargetGL::getCubeMap()
{
	return &cubeMapResult;
}

GLuint CubeRenderTargetGL::getRendertargetTexture()
{
	return renderTargetTexture;
}

void CubeRenderTargetGL::release()
{
	cubeMapResult.release();
	glDeleteFramebuffers(1, &frameBuffer);
	glDeleteRenderbuffers(1, &renderBuffer);
	glDeleteTextures(1, &renderTargetTexture);

	frameBuffer = GL_FALSE;
	renderBuffer = GL_FALSE;
	renderTargetTexture = GL_FALSE;

	cubeMapResult.release();
}

void CubeRenderTargetGL::setFrameBuffer(GLuint newValue)
{
	frameBuffer = newValue;
}

void CubeRenderTargetGL::setRenderBuffer(GLuint newValue)
{
	renderBuffer = newValue;
}

void CubeRenderTargetGL::setCubeMapResult(GLuint newValue)
{
	cubeMapResult = newValue;
}

void CubeRenderTargetGL::setRenderTargetTexture(GLuint newValue)
{
	renderTargetTexture = newValue;
}




RenderTargetGL::RenderTargetGL(int width, int height) : RenderTarget(width, height), frameBuffer(GL_FALSE), renderBuffer(GL_FALSE)
{
}

RenderTargetGL::~RenderTargetGL()
{
}

void RenderTargetGL::copyFrom(RenderTargetGL* dest, const Dimension& sourceDim, const Dimension& destDim)
{
	GLint readFBId = 0;
	GLint drawFboId = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFBId);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, dest->getFrameBuffer());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
	glBlitFramebuffer(sourceDim.xPos, sourceDim.yPos, sourceDim.width, sourceDim.height,
		destDim.xPos, destDim.yPos, destDim.width, destDim.height,
		GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
		GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFboId);
}

RenderTargetGL RenderTargetGL::createMultisampled(GLint textureChannel, int width, int height, 
	GLuint samples, GLuint depthStencilType)
{
	assert(samples > 1);

	RenderTargetGL result(width, height);
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

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return result;
}

RenderTargetGL RenderTargetGL::createSingleSampled(GLint internalFormat, int width, int height, GLint format,  GLint dataType, GLuint depthStencilType)
{
	RenderTargetGL result(width, height);
	result.width = width;
	result.height = height;

	glGenFramebuffers(1, &result.frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, result.frameBuffer);

	// Generate texture
	glGenTextures(1, &result.textureBuffer.textureID);
	const GLuint& textureID = result.textureBuffer.textureID;

	glBindTexture(GL_TEXTURE_2D, textureID);
	//GL_UNSIGNED_BYTE
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, dataType, nullptr);

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

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return result;
}

RenderTargetGL RenderTargetGL::createVSM(int width, int height)
{
	RenderTargetGL result(width, height);
	GLuint* frameBuffer = &result.frameBuffer;
	GLuint* textureID = &result.textureBuffer.textureID;
	TextureGL& texture = result.textureBuffer;
	glGenFramebuffers(1, frameBuffer);
	glGenTextures(1, textureID);


	//GL_RG32F
	glBindTexture(GL_TEXTURE_2D, *textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 2.0f, 2.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, *frameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *textureID, 0);


	//glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureID);
	//glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_RG32F, width, height, GL_TRUE);

	RendererOpenGL::checkGLErrors(BOOST_CURRENT_FUNCTION);



	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers


								   //create a render buffer for depth and stencil testing
	glGenRenderbuffers(1, &result.renderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, result.renderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// attach render buffer to the frame buffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, result.renderBuffer);
								   // Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw runtime_error("VarianceShadowMapGL::VarianceShadowMapGL(): Couldn't configure frame buffer!");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	RendererOpenGL::checkGLErrors(BOOST_CURRENT_FUNCTION);

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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
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

	RendererOpenGL::checkGLErrors(BOOST_CURRENT_FUNCTION);
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
