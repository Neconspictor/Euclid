#include <nex/opengl/texture/RenderTargetGL.hpp>
#include <cassert>
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <glm/gtc/matrix_transform.inl>

using namespace std;
using namespace glm;

nex::BaseRenderTargetGL::BaseRenderTargetGL(int width, int height, GLuint frameBuffer)
	: width(width), height(height), frameBuffer(frameBuffer)
{
}

nex::BaseRenderTargetGL::~BaseRenderTargetGL()
{
	if (frameBuffer != GL_FALSE) {
		GLCall(glDeleteFramebuffers(1, &frameBuffer));
		frameBuffer = GL_FALSE;
	}
}

nex::BaseRenderTargetGL::BaseRenderTargetGL(BaseRenderTargetGL && o) :
	frameBuffer(GL_FALSE)
{
	swap(o);
}

nex::BaseRenderTargetGL & nex::BaseRenderTargetGL::operator=(BaseRenderTargetGL && o)
{
	if (this == &o) return *this;
	swap(o);
	return *this;
}

void nex::BaseRenderTargetGL::copyFrom(BaseRenderTargetGL* dest, const Dimension& sourceDim, int components)
{
	GLint readFBId = 0;
	GLint drawFboId = 0;
	GLCall(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId));
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFBId);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, dest->getFrameBuffer());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
	glBlitFramebuffer(sourceDim.xPos, sourceDim.yPos, sourceDim.width, sourceDim.height,
		sourceDim.xPos, sourceDim.yPos, sourceDim.width, sourceDim.height,
		components,
		GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBId);
	GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFboId));
}

GLuint nex::BaseRenderTargetGL::getFrameBuffer()
{
	return frameBuffer;
}

void nex::BaseRenderTargetGL::setFrameBuffer(GLuint newValue)
{
	frameBuffer = newValue;
}

void nex::BaseRenderTargetGL::swap(BaseRenderTargetGL & o)
{
	frameBuffer = o.frameBuffer;
	width = o.width;
	height = o.height;
	o.frameBuffer = GL_FALSE;
}





nex::CubeRenderTargetGL::CubeRenderTargetGL(int width, int height, TextureData data) :
	BaseRenderTargetGL(width, height, GL_FALSE),
	renderBuffer(GL_FALSE),
	data(data),
	cubeMapResult(new CubeMapGL())
{

	cubeMapResult->setWidth(width);
	cubeMapResult->setHeight(height);

	// generate framebuffer and renderbuffer with a depth component
	GLCall(glGenFramebuffers(1, &frameBuffer));
	glGenRenderbuffers(1, &renderBuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);

	//glBindRenderbuffer(GL_RENDERBUFFER, 0);

	GLuint uvTechnique = static_cast<GLuint>(data.uvTechnique);
	GLuint minFilter = static_cast<GLuint>(data.minFilter);
	GLuint magFilter = static_cast<GLuint>(data.magFilter);
	GLuint internalFormat = static_cast<GLuint>(data.internalFormat);
	GLuint colorspace = static_cast<GLuint>(data.colorspace);
	GLuint pixelDataType = static_cast<GLuint>(data.pixelDataType);


	//pre-allocate the six faces of the cubemap
	glGenTextures(1, &cubeMapResult->textureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapResult->textureID);
	for (int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, colorspace,
			pixelDataType, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, uvTechnique);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, uvTechnique);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, uvTechnique);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, magFilter);

	if (data.generateMipMaps)
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER,0));
}

nex::CubeRenderTargetGL::~CubeRenderTargetGL()
{
	if (frameBuffer != GL_FALSE)
		GLCall(glDeleteFramebuffers(1, &frameBuffer));
	if (renderBuffer != GL_FALSE)
		GLCall(glDeleteRenderbuffers(1, &renderBuffer));

	frameBuffer = GL_FALSE;
	renderBuffer = GL_FALSE;
}

CubeMapGL * nex::CubeRenderTargetGL::createCopy()
{

	//first create a new cube render target that we use to blit the content
	CubeRenderTargetGL copy(width, height, data);

	GLint readFBId = 0;
	GLint drawFboId = 0;
	GLCall(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId));
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFBId);


	for (int i = 0; i < 6; ++i) {

		//attach the cubemap side of this render target
		glBindFramebuffer(GL_FRAMEBUFFER,  frameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubeMapResult->textureID, 0);

		//attach the cubemap side of the copy render target
		glBindFramebuffer(GL_FRAMEBUFFER, copy.frameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, copy.cubeMapResult->textureID, 0);

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// now we can blit the content to the copy
		glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, copy.frameBuffer);
		glBlitFramebuffer(0, 0, width, height,
			0, 0, copy.width, copy.height,
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
			GL_NEAREST);
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBId);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFboId);

	//extract cubemap texture from the copy and delete copy
	glBindFramebuffer(GL_FRAMEBUFFER, copy.frameBuffer);
	for (int i = 0; i < 6; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0); // unbound the cubemap side
	}

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	// reset the texture id of the cubemap of the copy, so that it won't be released!
	auto cache = copy.cubeMapResult;
	copy.cubeMapResult = nullptr;

	copy.release();

	return cache;
	
}

GLuint nex::CubeRenderTargetGL::getRenderBuffer()
{
	return renderBuffer;
}

GLuint nex::CubeRenderTargetGL::getCubeMapGL()
{
	return cubeMapResult->getCubeMap();
}

CubeMapGL * nex::CubeRenderTargetGL::getCubeMap()
{
	return cubeMapResult;
}

void nex::CubeRenderTargetGL::resizeForMipMap(unsigned int mipMapLevel) {

	if (!data.generateMipMaps) {
		throw_with_trace(runtime_error("CubeRenderTargetGL::resizeForMipMap(unsigned int): No mip levels generated for this cube rener target!"));
	}

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer));
	glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);

	unsigned int mipWidth = (unsigned int)(width * std::pow(0.5, mipMapLevel));
	unsigned int mipHeight = (unsigned int)(height * std::pow(0.5, mipMapLevel));
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);

	//glBindRenderbuffer(GL_RENDERBUFFER, 0);
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void nex::CubeRenderTargetGL::setRenderBuffer(GLuint newValue)
{
	renderBuffer = newValue;
}

void nex::CubeRenderTargetGL::setCubeMap(CubeMapGL* cubeMap)
{
	cubeMapResult = cubeMap;
}

void nex::CubeRenderTargetGL::setCubeMapResult(GLuint newValue)
{
	*cubeMapResult = newValue;
}

nex::RenderTargetGL::RenderTargetGL(int width, int height) :
	BaseRenderTargetGL(width, height, GL_FALSE),
	renderBuffer(GL_FALSE)
{
	textureBuffer = new TextureGL();
	textureBuffer->setWidth(width);
	textureBuffer->setHeight(height);
}

nex::RenderTargetGL::~RenderTargetGL()
{
	release();
}

RenderTargetGL* nex::RenderTargetGL::createMultisampled(int width, int height, const TextureData& data,
	GLuint samples, GLuint depthStencilType)
{
	assert(samples > 1);


	nex::Guard<RenderTargetGL> result;
	result.setContent(new RenderTargetGL(width, height));
	result->width = width;
	result->height = height;

	GLuint uvTechnique = static_cast<GLuint>(data.uvTechnique);
	GLuint minFilter = static_cast<GLuint>(data.minFilter);
	GLuint magFilter = static_cast<GLuint>(data.magFilter);
	GLuint internalFormat = static_cast<GLuint>(data.internalFormat);

	GLCall(glGenFramebuffers(1, &result->frameBuffer));
	glBindFramebuffer(GL_FRAMEBUFFER, result->frameBuffer);

	// Generate texture
	glGenTextures(1, &result->textureBuffer->textureID);
	const GLuint& textureID = result->textureBuffer->textureID;

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureID);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_TRUE);

	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, magFilter);

	// clamp is important so that no pixel artifacts occur on the border!
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, uvTechnique);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, uvTechnique);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	// attach texture to currently bound frame buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureID, 0);

	//create a render buffer for depth and stencil testing
	glGenRenderbuffers(1, &result->renderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, result->renderBuffer);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, depthStencilType, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// attach render buffer to the frame buffer
	if (depthStencilType == GL_DEPTH_COMPONENT)
	{
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, result->renderBuffer);
	}
	else
	{
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, result->renderBuffer);
	}

	// finally check if all went successfully
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw_with_trace(runtime_error("RendererOpenGL::createRenderTarget(): Couldn't successfully init framebuffer!"));
	}

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	auto cache = result.get();
	result.setContent(nullptr);

	return cache;
}

RenderTargetGL* nex::RenderTargetGL::createSingleSampled(int width, int height, const TextureData& data, GLuint depthStencilType)
{
	nex::Guard<RenderTargetGL> result;
	result.setContent(new RenderTargetGL(width, height));
	result->width = width;
	result->height = height;

	GLuint uvTechnique = static_cast<GLuint>(data.uvTechnique);
	GLuint minFilter = static_cast<GLuint>(data.minFilter);
	GLuint magFilter = static_cast<GLuint>(data.magFilter);
	GLuint internalFormat = static_cast<GLuint>(data.internalFormat);
	GLuint colorspace = static_cast<GLuint>(data.colorspace);
	GLuint pixelDataType = static_cast<GLuint>(data.pixelDataType);

	GLCall(glGenFramebuffers(1, &result->frameBuffer));
	glBindFramebuffer(GL_FRAMEBUFFER, result->frameBuffer);

	// Generate texture
	glGenTextures(1, &result->textureBuffer->textureID);
	const GLuint& textureID = result->textureBuffer->textureID;


	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	//glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, colorspace, pixelDataType, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	// clamp is important so that no pixel artifacts occur on the border!
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, uvTechnique);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, uvTechnique);

	//glBindTexture(GL_TEXTURE_2D, 0);

	//create a render buffer for depth and stencil testing
	glGenRenderbuffers(1, &result->renderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, result->renderBuffer);

	// GL_DEPTH_COMPONENT24 depthStencilType
	glRenderbufferStorage(GL_RENDERBUFFER, depthStencilType, width, height);
	//glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// attach texture to currently bound frame buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers


	// attach render buffer to the frame buffer
	if (depthStencilType == GL_DEPTH_COMPONENT)
	{
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, result->renderBuffer);
	}
	else
	{
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, result->renderBuffer);
	}

	// finally check if all went successfully
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw_with_trace(runtime_error("RenderTargetGL::createSingleSampled(): Couldn't successfully init framebuffer!"));
	}

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	auto cache = result.get();
	result.setContent(nullptr);

	return cache;
}

RenderTargetGL* nex::RenderTargetGL::createVSM(int width, int height)
{
	nex::Guard<RenderTargetGL> result;
	result.setContent(new RenderTargetGL(width, height));
	result->width = width;
	result->height = height;

	GLuint* frameBuffer = &result->frameBuffer;
	GLuint* textureID = &result->textureBuffer->textureID;
	TextureGL& texture = *result->textureBuffer;
	GLCall(glGenFramebuffers(1, frameBuffer));
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
	glGenRenderbuffers(1, &result->renderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, result->renderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// attach render buffer to the frame buffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, result->renderBuffer);
								   // Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw_with_trace(runtime_error("VarianceShadowMapGL::VarianceShadowMapGL(): Couldn't configure frame buffer!"));
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	RendererOpenGL::checkGLErrors(BOOST_CURRENT_FUNCTION);

	auto cache = result.get();
	result.setContent(nullptr);

	return cache;
}

GLuint nex::RenderTargetGL::getRenderBuffer()
{
	return renderBuffer;
}

GLuint nex::RenderTargetGL::getTextureGL()
{
	return textureBuffer->getTexture();
}

TextureGL* nex::RenderTargetGL::getTexture()
{
	return &*textureBuffer;
}

void nex::RenderTargetGL::release()
{
	//textureBuffer.release();

	if (frameBuffer != GL_FALSE)
		GLCall(glDeleteFramebuffers(1, &frameBuffer));
	if (renderBuffer != GL_FALSE)
		GLCall(glDeleteRenderbuffers(1, &renderBuffer));

	frameBuffer = GL_FALSE;
	renderBuffer = GL_FALSE;
}

void nex::RenderTargetGL::setRenderBuffer(GLuint newValue)
{
	renderBuffer = newValue;
}

void nex::RenderTargetGL::setTexture(TextureGL* texture)
{
	textureBuffer = texture;
}

void nex::RenderTargetGL::setTextureBuffer(GLuint newValue)
{
	textureBuffer->setTexture(newValue);
}

nex::CubeDepthMapGL::CubeDepthMapGL(int width, int height) :
	BaseRenderTargetGL(width, height, GL_FALSE)
{
	GLuint texture;
	GLCall(glGenTextures(1, &texture));
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
		throw_with_trace(runtime_error("CubeDepthMapGL::CubeDepthMapGL(int, int): Framebuffer not complete!"));

	// A depth map only needs depth (z-value) informations; therefore disable any color buffers
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

nex::CubeDepthMapGL::~CubeDepthMapGL()
{
}

GLuint nex::CubeDepthMapGL::getCubeMapTexture() const
{
	return cubeMap.getCubeMap();
}

CubeMapGL* nex::CubeDepthMapGL::getCubeMap()
{
	return &cubeMap;
}

GLuint nex::CubeDepthMapGL::getFramebuffer() const
{
	return frameBuffer;
}

nex::DepthMapGL::DepthMapGL(int width, int height) :
	BaseRenderTargetGL(width, height, GL_FALSE)
{
	GLuint textureID = GL_FALSE;
	GLCall(glGenFramebuffers(1, &frameBuffer));
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

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	RendererOpenGL::checkGLErrors(BOOST_CURRENT_FUNCTION);
}

nex::DepthMapGL::~DepthMapGL()
{
}

GLuint nex::DepthMapGL::getFramebuffer() const
{
	return frameBuffer;
}

GLuint nex::DepthMapGL::getTexture() const
{
	return texture.getTexture();
}

TextureGL* nex::DepthMapGL::getTexture()
{
	return &texture;
}

void nex::DepthMapGL::release()
{
	//texture.release();
	if (frameBuffer != GL_FALSE)
		GLCall(glDeleteFramebuffers(1, &frameBuffer));
	frameBuffer = GL_FALSE;
}

nex::PBR_GBufferGL::PBR_GBufferGL(int width, int height)
	: 
	BaseRenderTargetGL(width, height, GL_FALSE),
	albedo(GL_FALSE),
	aoMetalRoughness(GL_FALSE),
	normal(GL_FALSE),
	position(GL_FALSE)
{
	GLCall(glGenFramebuffers(1, &frameBuffer));
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	//unsigned int gPosition, gNormal, gAlbedo;
	unsigned int tempTexture;

	// albedo
	glGenTextures(1, &tempTexture);
	albedo.setTexture(tempTexture);

	glBindTexture(GL_TEXTURE_2D, tempTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tempTexture, 0);

	// ao metal roughness
	glGenTextures(1, &tempTexture);
	aoMetalRoughness.setTexture(tempTexture);

	glBindTexture(GL_TEXTURE_2D, tempTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tempTexture, 0);

	// normal
	glGenTextures(1, &tempTexture);
	normal.setTexture(tempTexture);

	glBindTexture(GL_TEXTURE_2D, tempTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, tempTexture, 0);

	// position
	glGenTextures(1, &tempTexture);
	position.setTexture(tempTexture);

	glBindTexture(GL_TEXTURE_2D, tempTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, tempTexture, 0);



	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, 
		GL_COLOR_ATTACHMENT1, 
		GL_COLOR_ATTACHMENT2, 
		GL_COLOR_ATTACHMENT3
	};

	glDrawBuffers(4, attachments);

	// create and attach depth buffer (renderbuffer)
	/*unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	depth.setTexture(rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepth);*/

	// depth/stencil
	glGenTextures(1, &tempTexture);
	depth.setTexture(tempTexture);

	glBindTexture(GL_TEXTURE_2D, tempTexture);
	//glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, width, height);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, 0);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, tempTexture, 0);

	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw_with_trace(std::runtime_error("PBR_DeferredGL::createMultipleRenderTarget(int, int): Couldn't successfully init framebuffer!"));
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	
}

TextureGL * nex::PBR_GBufferGL::getAlbedo()
{
	return &albedo;
}

TextureGL * nex::PBR_GBufferGL::getAoMetalRoughness()
{
	return &aoMetalRoughness;
}

TextureGL * nex::PBR_GBufferGL::getNormal()
{
	return &normal;
}

TextureGL * nex::PBR_GBufferGL::getPosition()
{
	return &position;
}

TextureGL * nex::PBR_GBufferGL::getDepth()
{
	return &depth;
}

nex::OneTextureRenderTarget::OneTextureRenderTarget(GLuint frameBuffer,
	TextureGL texture,
	unsigned int width,
	unsigned int height) :
	BaseRenderTargetGL(width, height, frameBuffer),
	m_texture(move(texture))
{
}

nex::OneTextureRenderTarget::~OneTextureRenderTarget()
{
}

TextureGL * nex::OneTextureRenderTarget::getTexture()
{
	return &m_texture;
}