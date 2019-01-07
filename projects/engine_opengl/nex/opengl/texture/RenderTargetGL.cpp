#include <nex/opengl/texture/RenderTargetGL.hpp>
#include <cassert>
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <glm/gtc/matrix_transform.inl>

using namespace std;
using namespace glm;

nex::CubeRenderTarget* nex::CubeRenderTarget::createSingleSampled(int width, int height, const TextureData& data,
	DepthStencil depthStencilType)
{
	return new CubeRenderTarget(width, height, data);
}

void nex::CubeRenderTarget::resizeForMipMap(unsigned mipMapLevel)
{
	CubeRenderTargetGL* gl = (CubeRenderTargetGL*)getImpl();
	gl->resizeForMipMap(mipMapLevel);
}

nex::CubeRenderTarget::CubeRenderTarget(int width, int height, TextureData data) : 
	RenderTarget(new CubeRenderTargetGL(width, height, data))
{

}


nex::CubeRenderTargetGL::CubeRenderTargetGL(int width, int height, TextureData data) :
	RenderTargetGL(width, height), data(std::move(data))
{
	mRenderResult = CubeMap::create();
	mRenderResult->setWidth(width);
	mRenderResult->setHeight(height);

	// generate framebuffer and renderbuffer with a depth component
	GLCall(glGenFramebuffers(1, &frameBuffer));
	GLCall(glGenRenderbuffers(1, &renderBuffer));

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer));

	GLCall(glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer));
	GLCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height));
	GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer));

	//glBindRenderbuffer(GL_RENDERBUFFER, 0);

	GLuint wrapR = translate(data.wrapR);
	GLuint wrapS = translate(data.wrapS);
	GLuint wrapT = translate(data.wrapT);
	GLuint minFilter = translate(data.minFilter);
	GLuint magFilter = translate(data.magFilter);
	GLuint internalFormat = translate(data.internalFormat);
	GLuint colorspace = translate(data.colorspace);
	GLuint pixelDataType = translate(data.pixelDataType);


	//pre-allocate the six faces of the cubemap
	TextureGL* textureGL = (TextureGL*)mRenderResult->getImpl();
	GLCall(glGenTextures(1, textureGL->getTexture()));
	GLCall(glActiveTexture(GL_TEXTURE0));
	GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, *textureGL->getTexture()));
	for (int i = 0; i < 6; ++i)
	{
		GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, colorspace,
			pixelDataType, nullptr));
	}

	GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapR));
	GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapS));
	GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapT));
	GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilter));
	GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, magFilter));

	if (data.generateMipMaps)
		GLCall(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));

	GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
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

nex::CubeMapGL * nex::CubeRenderTargetGL::createCopy()
{

	//first create a new cube render target that we use to blit the content
	CubeRenderTargetGL copy(width, height, data);

	GLint readFBId = 0;
	GLint drawFboId = 0;
	GLCall(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId));
	GLCall(glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFBId));


	TextureGL* textureGL = (TextureGL*)mRenderResult->getImpl();
	TextureGL* copyTexGL = (TextureGL*)copy.mRenderResult->getImpl();

	for (int i = 0; i < 6; ++i) {

		//attach the cubemap side of this render target
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER,  frameBuffer));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, *textureGL->getTexture(), 0));

		//attach the cubemap side of the copy render target
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, copy.frameBuffer));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, *copyTexGL->getTexture(), 0));

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// now we can blit the content to the copy
		GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer));
		GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, copy.frameBuffer));
		GLCall(glBlitFramebuffer(0, 0, width, height,
			0, 0, copy.width, copy.height,
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
			GL_NEAREST));
	}

	GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBId));
	GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFboId));

	//extract cubemap texture from the copy and delete copy
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, copy.frameBuffer));
	for (int i = 0; i < 6; ++i) {
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0)); // unbound the cubemap side
	}

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	// reset the texture id of the cubemap of the copy, so that it won't be released!
	auto cache = copy.mRenderResult.get();
	copy.mRenderResult = nullptr;

	copy.release();

	return (CubeMapGL*)cache;
	
}


void nex::CubeRenderTargetGL::resizeForMipMap(unsigned int mipMapLevel) {

	if (!data.generateMipMaps) {
		throw_with_trace(runtime_error("CubeRenderTargetGL::resizeForMipMap(unsigned int): No mip levels generated for this cube rener target!"));
	}

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer));
	GLCall(glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer));

	unsigned int mipWidth = (unsigned int)(width * std::pow(0.5, mipMapLevel));
	unsigned int mipHeight = (unsigned int)(height * std::pow(0.5, mipMapLevel));
	GLCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight));
	GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer));

	//glBindRenderbuffer(GL_RENDERBUFFER, 0);
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

nex::RenderTargetGL::RenderTargetGL(int width, int height, GLuint frameBuffer) : RenderTargetImpl(width, height),
                                                             width(width), height(height),
                                                             renderBuffer(GL_FALSE),
                                                             frameBuffer(frameBuffer)
{
}

nex::RenderTargetGL::~RenderTargetGL()
{
	release();
}

void nex::RenderTarget::copyFrom(RenderTarget* dest, const Dimension& sourceDim, int components)
{

	RenderTargetGL* self = (RenderTargetGL*)mImpl;
	RenderTargetGL* other = (RenderTargetGL*)dest->mImpl;

	GLint readFBId = 0;
	GLint drawFboId = 0;
	GLCall(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId));
	GLCall(glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFBId));

	GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, other->getFrameBuffer()));
	GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, self->getFrameBuffer()));
	GLCall(glBlitFramebuffer(sourceDim.xPos, sourceDim.yPos, sourceDim.width, sourceDim.height,
		sourceDim.xPos, sourceDim.yPos, sourceDim.width, sourceDim.height,
		components,
		GL_NEAREST));
	GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBId));
	GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFboId));
}


nex::RenderTarget* nex::RenderTarget::createMultisampled(int width, int height, const TextureData& data,
                                                             GLuint samples, DepthStencil depthStencilType)
{
	assert(samples > 1);


	nex::Guard<RenderTargetGL> glTarget;
	glTarget.setContent(new RenderTargetGL(width, height));
	glTarget->width = width;
	glTarget->height = height;

	GLuint wrapR = translate(data.wrapR);
	GLuint wrapS = translate(data.wrapS);
	GLuint wrapT = translate(data.wrapT);
	GLuint minFilter = translate(data.minFilter);
	GLuint magFilter = translate(data.magFilter);
	GLuint internalFormat = translate(data.internalFormat);
	GLuint depthStencilTypeGL = translate(depthStencilType);

	GLCall(glGenFramebuffers(1, &glTarget->frameBuffer));
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, glTarget->frameBuffer));

	// Generate texture
	TextureGL* textureGL = (TextureGL*)glTarget->mRenderResult->getImpl();
	GLCall(glGenTextures(1, textureGL->getTexture()));
	const GLuint& textureID = *textureGL->getTexture();

	GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureID));
	GLCall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_TRUE));

	GLCall(glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, minFilter));
	GLCall(glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, magFilter));

	// clamp is important so that no pixel artifacts occur on the border!
	GLCall(glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_R, wrapR));
	GLCall(glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, wrapS));
	GLCall(glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, wrapT));

	GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0));

	// attach texture to currently bound frame buffer
	GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureID, 0));


	if (depthStencilType != DepthStencil::NONE)
	{

		//create a render buffer for depth and stencil testing
		GLCall(glGenRenderbuffers(1, &glTarget->renderBuffer));
		GLCall(glBindRenderbuffer(GL_RENDERBUFFER, glTarget->renderBuffer));
		GLCall(glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, depthStencilTypeGL, width, height));
		//glBindRenderbuffer(GL_RENDERBUFFER, 0);


		// attach render buffer to the frame buffer
		if (isNoStencilFormat(depthStencilType))
		{
			GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, glTarget->renderBuffer));
		}
		else
		{
			GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, glTarget->renderBuffer));
		}
	}

	// finally check if all went successfully
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw_with_trace(runtime_error("RendererOpenGL::createRenderTarget(): Couldn't successfully init framebuffer!"));
	}

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	RenderTarget* result = new RenderTarget(glTarget.get());
	glTarget.reset();

	return result;
}

nex::RenderTarget* nex::RenderTarget::createSingleSampled(int width, int height, const TextureData& data, DepthStencil depthStencilType)
{
	nex::Guard<RenderTargetGL> glTarget;
	glTarget.setContent(new RenderTargetGL(width, height));
	glTarget->width = width;
	glTarget->height = height;

	GLuint wrapR = translate(data.wrapR);
	GLuint wrapS = translate(data.wrapS);
	GLuint wrapT = translate(data.wrapT);
	GLuint minFilter = translate(data.minFilter);
	GLuint magFilter = translate(data.magFilter);
	GLuint internalFormat = translate(data.internalFormat);
	GLuint colorspace = translate(data.colorspace);
	GLuint pixelDataType = translate(data.pixelDataType);
	GLuint depthStencilTypeGL = translate(depthStencilType);

	GLCall(glGenFramebuffers(1, &glTarget->frameBuffer));
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, glTarget->frameBuffer));

	// Generate texture

	TextureGL* textureGL = (TextureGL*)glTarget->getTexture()->getImpl();
	GLCall(glGenTextures(1, textureGL->getTexture()));
	const GLuint& textureID = *textureGL->getTexture();


	//glActiveTexture(GL_TEXTURE0);
	GLCall(glBindTexture(GL_TEXTURE_2D, textureID));
	//glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, colorspace, pixelDataType, 0));

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter));

	// clamp is important so that no pixel artifacts occur on the border!
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, wrapR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT));

	//swizzle
	if (data.useSwizzle)
	{
		int swizzle [4];
		swizzle[0] = translate(data.swizzle.r);
		swizzle[1] = translate(data.swizzle.g);
		swizzle[2] = translate(data.swizzle.b);
		swizzle[3] = translate(data.swizzle.a);

		GLCall(glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle));

	}


	//glBindTexture(GL_TEXTURE_2D, 0);

	// attach texture to currently bound frame buffer
	GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0));

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	GLCall(glDrawBuffers(1, DrawBuffers)); // "1" is the size of DrawBuffers


	if (depthStencilType != DepthStencil::NONE)
	{
		//create a render buffer for depth and stencil testing
		GLCall(glGenRenderbuffers(1, &glTarget->renderBuffer));
		GLCall(glBindRenderbuffer(GL_RENDERBUFFER, glTarget->renderBuffer));
		GLCall(glRenderbufferStorage(GL_RENDERBUFFER, depthStencilTypeGL, width, height));
		//glBindRenderbuffer(GL_RENDERBUFFER, 0);


		// attach render buffer to the frame buffer
		if (isNoStencilFormat(depthStencilType))
		{
			GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, glTarget->renderBuffer));
		}
		else
		{
			GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, glTarget->renderBuffer));
		}
	}

	// finally check if all went successfully
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw_with_trace(runtime_error("RenderTargetGL::createSingleSampled(): Couldn't successfully init framebuffer!"));
	}

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	RenderTarget* result = new RenderTarget(glTarget.get());
	glTarget.reset();

	return result;
}

nex::RenderTarget::RenderTarget(RenderTargetImpl* impl) : mImpl(impl)
{
}

void nex::RenderTarget::bind()
{
	RenderTargetGL* impl = (RenderTargetGL*)mImpl;
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, impl->getFrameBuffer()));
}

int nex::RenderTarget::getHeight() const
{
	RenderTargetGL* impl = (RenderTargetGL*)mImpl;
	return impl->height;
}

int nex::RenderTarget::getWidth() const
{
	RenderTargetGL* impl = (RenderTargetGL*)mImpl;
	return impl->width;
}

void nex::RenderTarget::unbind()
{
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

nex::RenderTarget* nex::RenderTarget::createVSM(int width, int height)
{
	nex::Guard<RenderTargetGL> result;
	result.setContent(new RenderTargetGL(width, height));
	result->width = width;
	result->height = height;

	GLuint* frameBuffer = &result->frameBuffer;

	TextureGL& texture = *(TextureGL*)result->getTexture()->getImpl();
	GLuint* textureID = texture.getTexture();
	GLCall(glGenFramebuffers(1, frameBuffer));
	GLCall(glGenTextures(1, textureID));


	//GL_RG32F
	GLCall(glBindTexture(GL_TEXTURE_2D, *textureID));
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, 0));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
	GLfloat borderColor[] = { 2.0f, 2.0f };
	GLCall(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor));
	GLCall(glGenerateMipmap(GL_TEXTURE_2D));
	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, *frameBuffer));
	GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *textureID, 0));


	//glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureID);
	//glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_RG32F, width, height, GL_TRUE);

	RendererOpenGL::checkGLErrors(BOOST_CURRENT_FUNCTION);



	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	GLCall(glDrawBuffers(1, DrawBuffers)); // "1" is the size of DrawBuffers


								   //create a render buffer for depth and stencil testing
	GLCall(glGenRenderbuffers(1, &result->renderBuffer));
	GLCall(glBindRenderbuffer(GL_RENDERBUFFER, result->renderBuffer));
	GLCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height));
	GLCall(glBindRenderbuffer(GL_RENDERBUFFER, 0));

	// attach render buffer to the frame buffer
	GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, result->renderBuffer));
								   // Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw_with_trace(runtime_error("VarianceShadowMapGL::VarianceShadowMapGL(): Couldn't configure frame buffer!"));
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	RendererOpenGL::checkGLErrors(BOOST_CURRENT_FUNCTION);

	RenderTarget* target = new RenderTarget(result.get());
	result.reset();

	return target;
}


GLuint nex::RenderTargetGL::getFrameBuffer() const
{
	return frameBuffer;
}

GLuint nex::RenderTargetGL::getRenderBuffer()
{
	return renderBuffer;
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

void nex::RenderTargetGL::setFrameBuffer(GLuint newValue)
{
	frameBuffer = newValue;
}

void nex::RenderTargetGL::setRenderBuffer(GLuint newValue)
{
	renderBuffer = newValue;
}

nex::CubeDepthMap* nex::CubeDepthMap::create(unsigned width, unsigned height)
{
	return new CubeDepthMap(width, height);
}

nex::CubeDepthMap::CubeDepthMap(int width, int height) : RenderTarget(new CubeDepthMapGL(width, height))
{
}

nex::CubeDepthMapGL::CubeDepthMapGL(int width, int height) :
	RenderTargetGL(width, height)
{

	mRenderResult = CubeMap::create();
	mRenderResult->setWidth(width);
	mRenderResult->setHeight(height);

	GLuint texture;
	GLCall(glGenTextures(1, &texture));
	glGenFramebuffers(1, &frameBuffer);
	TextureGL* textureGL = (TextureGL*)mRenderResult->getImpl();
	textureGL->setTexture(texture);

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

nex::DepthMap* nex::DepthMap::create(unsigned width, unsigned height)
{
	return new DepthMap(width, height);
}

nex::DepthMap::DepthMap(int width, int height) : RenderTarget(new DepthMapGL(width, height))
{
}


nex::DepthMapGL::DepthMapGL(int width, int height) :
	RenderTargetGL(width, height)
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

nex::PBR_GBuffer* nex::PBR_GBuffer::create(unsigned width, unsigned height)
{
	return new PBR_GBuffer(width, height);
}

nex::PBR_GBuffer::PBR_GBuffer(int width, int height) : RenderTarget(new PBR_GBufferGL(width, height))
{
}

nex::Texture* nex::PBR_GBuffer::getAlbedo() const
{
	PBR_GBufferGL* gl = (PBR_GBufferGL*)getImpl();
	return gl->getAlbedo();
}

nex::Texture* nex::PBR_GBuffer::getAoMetalRoughness() const
{
	PBR_GBufferGL* gl = (PBR_GBufferGL*)getImpl();
	return gl->getAoMetalRoughness();
}

nex::Texture* nex::PBR_GBuffer::getNormal() const
{
	PBR_GBufferGL* gl = (PBR_GBufferGL*)getImpl();
	return gl->getNormal();
}

nex::Texture* nex::PBR_GBuffer::getPosition() const
{
	PBR_GBufferGL* gl = (PBR_GBufferGL*)getImpl();
	return gl->getPosition();
}

nex::RenderBuffer* nex::PBR_GBuffer::getDepth() const
{
	PBR_GBufferGL* gl = (PBR_GBufferGL*)getImpl();
	return gl->getDepth();
}

nex::PBR_GBufferGL::PBR_GBufferGL(int width, int height)
	: 
	RenderTargetGL(width, height),
	albedo(Texture::create()),
	aoMetalRoughness(Texture::create()),
	normal(Texture::create()),
	position(Texture::create()),
	depth(RenderBuffer::create())
{
	GLCall(glGenFramebuffers(1, &frameBuffer));
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	//unsigned int gPosition, gNormal, gAlbedo;
	unsigned int tempTexture;

	// albedo
	glGenTextures(1, &tempTexture);
	((TextureGL*)albedo->getImpl())->setTexture(tempTexture);

	glBindTexture(GL_TEXTURE_2D, tempTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tempTexture, 0);

	// ao metal roughness
	glGenTextures(1, &tempTexture);
	((TextureGL*)aoMetalRoughness->getImpl())->setTexture(tempTexture);

	glBindTexture(GL_TEXTURE_2D, tempTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tempTexture, 0);

	// normal
	glGenTextures(1, &tempTexture);
	((TextureGL*)normal->getImpl())->setTexture(tempTexture);

	glBindTexture(GL_TEXTURE_2D, tempTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, tempTexture, 0);

	// position
	glGenTextures(1, &tempTexture);
	((TextureGL*)position->getImpl())->setTexture(tempTexture);

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
	glGenTextures(1, &renderBuffer);
	((RenderBufferGL*)depth->getImpl())->setTexture(renderBuffer);

	glBindTexture(GL_TEXTURE_2D, renderBuffer);
	//glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, width, height);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, 0);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, renderBuffer, 0);

	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw_with_trace(std::runtime_error("PBR_DeferredGL::createMultipleRenderTarget(int, int): Couldn't successfully init framebuffer!"));
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	
}

nex::Texture* nex::PBR_GBufferGL::getAlbedo()
{
	return albedo.get();
}

nex::Texture * nex::PBR_GBufferGL::getAoMetalRoughness()
{
	return aoMetalRoughness.get();
}

nex::Texture * nex::PBR_GBufferGL::getNormal()
{
	return normal.get();
}

nex::Texture* nex::PBR_GBufferGL::getPosition()
{
	return position.get();
}

nex::RenderBuffer * nex::PBR_GBufferGL::getDepth()
{
	return depth.get();
}