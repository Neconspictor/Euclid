#include <nex/opengl/texture/TextureGL.hpp>
#include <memory>
#include <cassert>
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <nex/opengl/texture/TextureManagerGL.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <glm/gtc/matrix_transform.inl>

using namespace std;
using namespace glm;

mat4 CubeMapGL::rightSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f));
mat4 CubeMapGL::leftSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f));
mat4 CubeMapGL::topSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));
mat4 CubeMapGL::bottomSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f));
mat4 CubeMapGL::frontSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f));
mat4 CubeMapGL::backSide = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f));



const mat4& CubeMapGL::getViewLookAtMatrixRH(Side side)
{
	switch (side) {
	case POSITIVE_X:
		return rightSide;
	case NEGATIVE_X:
		return leftSide;
	case POSITIVE_Y:
		return topSide;
	case NEGATIVE_Y:
		return bottomSide;
	case NEGATIVE_Z:
		return frontSide;
	case POSITIVE_Z:
		return backSide;
	default:
		throw_with_trace(std::runtime_error("No mapping defined for " + side));
	}

	// won't be reached
	return rightSide;
}

CubeMapGL::CubeMapGL() : TextureGL() {}

CubeMapGL::CubeMapGL(GLuint cubeMap) : TextureGL(cubeMap){}

void CubeMapGL::generateMipMaps()
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}


GLuint CubeMapGL::getCubeMap() const
{
	return textureID;
}

void CubeMapGL::setCubeMap(GLuint id)
{
	textureID = id;
}

bool nex::Texture::isCubeTarget(TextureTarget target)
{
	auto first  = static_cast<unsigned>(TextureTarget::CUBE_POSITIVE_X);
	auto last = static_cast<unsigned>(TextureTarget::CUBE_NEGATIVE_Z);
	auto current = static_cast<unsigned>(target);
	return first <= current && last >= current;
}

TextureGL::TextureGL(): textureID(GL_FALSE)
{
}

TextureGL::TextureGL(GLuint texture) : textureID(texture)
{
}

TextureGL::TextureGL(TextureGL && o) : textureID(o.textureID)
{
	o.textureID = GL_FALSE;
}

TextureGL & TextureGL::operator=(TextureGL && o)
{
	if (this == &o) return *this;
	textureID = o.textureID;
	o.textureID = GL_FALSE;
	return *this;
}

TextureGL::~TextureGL()
{
	release();
}

TextureGL* TextureGL::createFromImage(const StoreImageGL& store, const TextureData& data, bool isCubeMap)
{
	GLuint format = static_cast<GLuint>(data.colorspace);
	GLuint internalFormat = static_cast<GLuint>(data.internalFormat);
	GLuint pixelDataType = static_cast<GLuint>(data.pixelDataType);
	GLuint bindTarget;

	if (isCubeMap)
	{
		assert(store.sideCount == 6);
		bindTarget = GL_TEXTURE_CUBE_MAP;
	}
	else
	{
		assert(store.sideCount == 1);
		bindTarget = GL_TEXTURE_2D;
	}

	GLuint textureID;
	GLCall(glActiveTexture(GL_TEXTURE0));
	glGenTextures(1, &textureID);
	glBindTexture(bindTarget, textureID);

	if (isCubeMap)
	{
		for (unsigned int i = 0; i < store.sideCount; ++i)
		{
			for (unsigned mipMapLevel = 0; mipMapLevel < store.mipmapCounts[i]; ++mipMapLevel)
			{
				auto& image = store.images[i][mipMapLevel];
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mipMapLevel, internalFormat, image.width, image.height, 0, format, pixelDataType, image.pixels.get());
			}

		}
	}
	else
	{
		for (unsigned mipMapLevel = 0; mipMapLevel < store.mipmapCounts[0]; ++mipMapLevel)
		{
			auto& image = store.images[0][mipMapLevel];
			glTexImage2D(GL_TEXTURE_2D, mipMapLevel, internalFormat, image.width, image.height, 0, format, pixelDataType, image.pixels.get());
		}
	}

	GLint minFilter = static_cast<GLuint>(data.minFilter);
	GLint magFilter = static_cast<GLuint>(data.magFilter);
	GLint uvTechnique = static_cast<GLuint>(data.uvTechnique);

	glTexParameteri(bindTarget, GL_TEXTURE_WRAP_S, uvTechnique);
	glTexParameteri(bindTarget, GL_TEXTURE_WRAP_T, uvTechnique);
	glTexParameteri(bindTarget, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(bindTarget, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(bindTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);

	if (data.generateMipMaps)
		glGenerateMipmap(bindTarget);

	GLCall(glBindTexture(bindTarget, 0));

	TextureGL* result;

	if (isCubeMap)
	{
		result = new CubeMapGL(textureID);
	}
	else
	{
		result = new TextureGL(textureID);
	}

	return result;
}

void TextureGL::release()
{
	if (textureID != GL_FALSE) {
		GLCall(glDeleteTextures(1, &textureID));
		textureID = GL_FALSE;
	}
}

GLuint TextureGL::getTexture() const
{
	return textureID;
}

void TextureGL::setTexture(GLuint id)
{
	textureID = id;
}

GLuint TextureGL::getFormat(int numberComponents)
{
	switch (numberComponents) {
		case 4: return GL_RGBA;
		case 3: return GL_RGB;
		case 2: return GL_RG;
		case 1: return GL_RED;
		default: {
			throw_with_trace(runtime_error("TextureManagerGL::getFormat(int): Not supported number of components " + numberComponents));
		}
	}

	// won't be reached
	return GL_FALSE;
}

RenderBufferGL::RenderBufferGL() : TextureGL()
{
}

RenderBufferGL::~RenderBufferGL()
{
	release();
}

RenderBufferGL::RenderBufferGL(GLuint texture) : TextureGL(texture)
{
}

RenderBufferGL::RenderBufferGL(RenderBufferGL && o) : TextureGL(move(o))
{
}

RenderBufferGL & RenderBufferGL::operator=(RenderBufferGL && o)
{
	TextureGL::operator=(move(o));
	return *this;
}

void RenderBufferGL::release()
{
	if (textureID != GL_FALSE) {
		GLCall(glDeleteRenderbuffers(1, &textureID));
		textureID = GL_FALSE;
	}
}


BaseRenderTargetGL::BaseRenderTargetGL(int width, int height, GLuint frameBuffer)
	: width(width), height(height), frameBuffer(frameBuffer)
{
}

BaseRenderTargetGL::~BaseRenderTargetGL()
{
	if (frameBuffer != GL_FALSE) {
		GLCall(glDeleteFramebuffers(1, &frameBuffer));
		frameBuffer = GL_FALSE;
	}
}

BaseRenderTargetGL::BaseRenderTargetGL(BaseRenderTargetGL && o) :
	frameBuffer(GL_FALSE)
{
	swap(o);
}

BaseRenderTargetGL & BaseRenderTargetGL::operator=(BaseRenderTargetGL && o)
{
	if (this == &o) return *this;
	swap(o);
	return *this;
}

void BaseRenderTargetGL::copyFrom(BaseRenderTargetGL* dest, const Dimension& sourceDim, int components)
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

GLuint BaseRenderTargetGL::getFrameBuffer()
{
	return frameBuffer;
}

void BaseRenderTargetGL::setFrameBuffer(GLuint newValue)
{
	frameBuffer = newValue;
}

void BaseRenderTargetGL::swap(BaseRenderTargetGL & o)
{
	frameBuffer = o.frameBuffer;
	width = o.width;
	height = o.height;
	o.frameBuffer = GL_FALSE;
}





CubeRenderTargetGL::CubeRenderTargetGL(int width, int height, TextureData data) :
	BaseRenderTargetGL(width, height, GL_FALSE),
	renderBuffer(GL_FALSE),
	data(data)
{
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
	glGenTextures(1, &cubeMapResult.textureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapResult.textureID);
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

CubeRenderTargetGL::~CubeRenderTargetGL()
{
	// Don't release memory. Is needed!
	//release();
}

CubeMapGL * CubeRenderTargetGL::createCopy()
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
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubeMapResult.textureID, 0);

		//attach the cubemap side of the copy render target
		glBindFramebuffer(GL_FRAMEBUFFER, copy.frameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, copy.cubeMapResult.textureID, 0);

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
	unsigned int cache = copy.cubeMapResult.textureID;
	glBindFramebuffer(GL_FRAMEBUFFER, copy.frameBuffer);
	for (int i = 0; i < 6; ++i) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0); // unbound the cubemap side
	}

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	// reset the texture id of the cubemap of the copy, so that it won't be released!
	copy.cubeMapResult.textureID = GL_FALSE;

	copy.release();

	//register the cubeMap to the texture manager and return the result
	CubeMapGL result(cache);
	return TextureManagerGL::get()->addCubeMap(move(result));
}

GLuint CubeRenderTargetGL::getRenderBuffer()
{
	return renderBuffer;
}

GLuint CubeRenderTargetGL::getCubeMapGL()
{
	return cubeMapResult.getCubeMap();
}

CubeMapGL * CubeRenderTargetGL::getCubeMap()
{
	return &cubeMapResult;
}

void CubeRenderTargetGL::release()
{
	cubeMapResult.release();
	GLCall(glDeleteFramebuffers(1, &frameBuffer));
	GLCall(glDeleteRenderbuffers(1, &renderBuffer));

	frameBuffer = GL_FALSE;
	renderBuffer = GL_FALSE;

	cubeMapResult.release();
}

void CubeRenderTargetGL::resizeForMipMap(unsigned int mipMapLevel) {

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

void CubeRenderTargetGL::setRenderBuffer(GLuint newValue)
{
	renderBuffer = newValue;
}

void CubeRenderTargetGL::setCubeMapResult(GLuint newValue)
{
	cubeMapResult = newValue;
}

RenderTargetGL::RenderTargetGL(int width, int height) : 
	BaseRenderTargetGL(width, height, GL_FALSE),
	renderBuffer(GL_FALSE)
{
}

RenderTargetGL::~RenderTargetGL()
{
	release();
}

RenderTargetGL RenderTargetGL::createMultisampled(int width, int height, const TextureData& data,
	GLuint samples, GLuint depthStencilType)
{
	assert(samples > 1);

	RenderTargetGL result(width, height);

	GLuint uvTechnique = static_cast<GLuint>(data.uvTechnique);
	GLuint minFilter = static_cast<GLuint>(data.minFilter);
	GLuint magFilter = static_cast<GLuint>(data.magFilter);
	GLuint internalFormat = static_cast<GLuint>(data.internalFormat);

	GLCall(glGenFramebuffers(1, &result.frameBuffer));
	glBindFramebuffer(GL_FRAMEBUFFER, result.frameBuffer);

	// Generate texture
	glGenTextures(1, &result.textureBuffer.textureID);
	const GLuint& textureID = result.textureBuffer.textureID;

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
		throw_with_trace(runtime_error("RendererOpenGL::createRenderTarget(): Couldn't successfully init framebuffer!"));
	}

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	return move(result);
}

RenderTargetGL RenderTargetGL::createSingleSampled(int width, int height, const TextureData& data, GLuint depthStencilType)
{
	RenderTargetGL result(width, height);
	result.width = width;
	result.height = height;

	GLuint uvTechnique = static_cast<GLuint>(data.uvTechnique);
	GLuint minFilter = static_cast<GLuint>(data.minFilter);
	GLuint magFilter = static_cast<GLuint>(data.magFilter);
	GLuint internalFormat = static_cast<GLuint>(data.internalFormat);
	GLuint colorspace = static_cast<GLuint>(data.colorspace);
	GLuint pixelDataType = static_cast<GLuint>(data.pixelDataType);

	GLCall(glGenFramebuffers(1, &result.frameBuffer));
	glBindFramebuffer(GL_FRAMEBUFFER, result.frameBuffer);

	// Generate texture
	glGenTextures(1, &result.textureBuffer.textureID);
	const GLuint& textureID = result.textureBuffer.textureID;


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
	glGenRenderbuffers(1, &result.renderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, result.renderBuffer);

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
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, result.renderBuffer);
	}
	else
	{
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, result.renderBuffer);
	}

	// finally check if all went successfully
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw_with_trace(runtime_error("RenderTargetGL::createSingleSampled(): Couldn't successfully init framebuffer!"));
	}

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	return move(result);
}

RenderTargetGL RenderTargetGL::createVSM(int width, int height)
{
	RenderTargetGL result(width, height);
	GLuint* frameBuffer = &result.frameBuffer;
	GLuint* textureID = &result.textureBuffer.textureID;
	TextureGL& texture = result.textureBuffer;
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
	glGenRenderbuffers(1, &result.renderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, result.renderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// attach render buffer to the frame buffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, result.renderBuffer);
								   // Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw_with_trace(runtime_error("VarianceShadowMapGL::VarianceShadowMapGL(): Couldn't configure frame buffer!"));
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	RendererOpenGL::checkGLErrors(BOOST_CURRENT_FUNCTION);

	return move(result);
}

GLuint RenderTargetGL::getRenderBuffer()
{
	return renderBuffer;
}

GLuint RenderTargetGL::getTextureGL()
{
	return textureBuffer.getTexture();
}

TextureGL* RenderTargetGL::getTexture()
{
	return &textureBuffer;
}

void RenderTargetGL::release()
{
	//textureBuffer.release();

	if (frameBuffer != GL_FALSE)
		GLCall(glDeleteFramebuffers(1, &frameBuffer));
	if (renderBuffer != GL_FALSE)
		GLCall(glDeleteRenderbuffers(1, &renderBuffer));

	frameBuffer = GL_FALSE;
	renderBuffer = GL_FALSE;
}

void RenderTargetGL::setRenderBuffer(GLuint newValue)
{
	renderBuffer = newValue;
}

void RenderTargetGL::setTextureBuffer(GLuint newValue)
{
	textureBuffer.setTexture(newValue);
}

CubeDepthMapGL::CubeDepthMapGL(int width, int height) : 
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

/*CubeDepthMapGL::CubeDepthMapGL(const CubeDepthMapGL& other) : 
	BaseRenderTarget(width, height),
	BaseRenderTargetGl(width, height),
	TextureGL(other), 
	CubeDepthMap(other), 
	cubeMap(other.cubeMap),
	frameBuffer(other.frameBuffer)
{
}

CubeDepthMapGL::CubeDepthMapGL(CubeDepthMapGL&& other) : 
	BaseRenderTarget(width, height),
	BaseRenderTargetGl(width, height),
	TextureGL(other), 
	CubeDepthMap(other), 
	cubeMap(other.cubeMap),
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
}*/

CubeDepthMapGL::~CubeDepthMapGL()
{
}

GLuint CubeDepthMapGL::getCubeMapTexture() const
{
	return cubeMap.getCubeMap();
}

CubeMapGL* CubeDepthMapGL::getCubeMap()
{
	return &cubeMap;
}

GLuint CubeDepthMapGL::getFramebuffer() const
{
	return frameBuffer;
}

DepthMapGL::DepthMapGL(int width, int height) :
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

TextureGL* DepthMapGL::getTexture()
{
	return &texture;
}

void DepthMapGL::release()
{
	//texture.release();
	if (frameBuffer != GL_FALSE)
		GLCall(glDeleteFramebuffers(1, &frameBuffer));
	frameBuffer = GL_FALSE;
}

PBR_GBufferGL::PBR_GBufferGL(int width, int height) 
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

TextureGL * PBR_GBufferGL::getAlbedo()
{
	return &albedo;
}

TextureGL * PBR_GBufferGL::getAoMetalRoughness()
{
	return &aoMetalRoughness;
}

TextureGL * PBR_GBufferGL::getNormal()
{
	return &normal;
}

TextureGL * PBR_GBufferGL::getPosition()
{
	return &position;
}

TextureGL * PBR_GBufferGL::getDepth()
{
	return &depth;
}

OneTextureRenderTarget::OneTextureRenderTarget(GLuint frameBuffer,
	TextureGL texture,
	unsigned int width,
	unsigned int height) :
	BaseRenderTargetGL(width, height, frameBuffer),
	m_texture(move(texture))
{
}

OneTextureRenderTarget::~OneTextureRenderTarget()
{
}

TextureGL * OneTextureRenderTarget::getTexture()
{
	return &m_texture;
}