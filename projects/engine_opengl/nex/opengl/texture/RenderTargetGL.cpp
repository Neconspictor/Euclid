#include <nex/opengl/texture/RenderTargetGL.hpp>
#include <cassert>
#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <glm/gtc/matrix_transform.inl>

using namespace std;
using namespace glm;


GLuint nex::translate(RenderAttachment::Type type, unsigned attachIndex)
{
	static AttachmentTypeGL table [] = {
		ATTACHMENT_COLOR,
		ATTACHMENT_DEPTH,
		ATTACHMENT_STENCIL,
		ATTACHMNET_DEPTH_STENCIL
	};

	static const size_t size = (size_t)RenderAttachment::Type::LAST - (size_t)RenderAttachment::Type::FIRST + 1;
	static const size_t tableSize = sizeof(table) / sizeof(AttachmentTypeGL);
	static_assert(tableSize == size, "RenderAttachment and RenderAttachmentTypeGL don't match!");

	GLuint resultBase = table[(unsigned)type];

	if (resultBase == ATTACHMENT_COLOR)
		resultBase += attachIndex;

	return resultBase;
}


nex::CubeRenderTarget::CubeRenderTarget(int width, int height, TextureData data) : 
	RenderTarget(make_unique<CubeRenderTargetGL>(width, height, data))
{
}

void nex::CubeRenderTarget::useSide(CubeMap::Side side, unsigned mipLevel)
{
	((CubeRenderTargetGL*)getImpl())->useSide(side, mipLevel);
}

unsigned nex::CubeRenderTarget::getHeightMipLevel(unsigned mipMapLevel) const
{
	return ((CubeRenderTargetGL*)getImpl())->getHeightMipLevel(mipMapLevel);
}

unsigned nex::CubeRenderTarget::getWidthMipLevel(unsigned mipMapLevel) const
{
	return ((CubeRenderTargetGL*)getImpl())->getWidthMipLevel(mipMapLevel);
}

unsigned nex::CubeRenderTarget::getSideWidth() const
{
	return ((CubeRenderTargetGL*)getImpl())->getWidth();
}

unsigned nex::CubeRenderTarget::getSideHeight() const
{
	return ((CubeRenderTargetGL*)getImpl())->getHeight();
}

void nex::CubeRenderTarget::resizeForMipMap(unsigned mipMapLevel)
{
	CubeRenderTargetGL* gl = (CubeRenderTargetGL*)getImpl();
	gl->resizeForMipMap(mipMapLevel);
}


nex::CubeRenderTargetGL::CubeRenderTargetGL(unsigned width, unsigned height, TextureData data) :
	RenderTargetGL(width, height), data(data)
{
	mRenderResult = make_unique<CubeMap>(width, height);

	auto renderBuffer = make_shared<RenderBuffer>(width, height, DepthStencilFormat::DEPTH24);
	


	// generate framebuffer and renderbuffer with a depth component
	GLCall(glGenFramebuffers(1, &mFrameBuffer));
	//GLCall(glGenRenderbuffers(1, &renderBuffer));

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer));

	useDepthStencilMap(renderBuffer);

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
	CubeMapGL* textureGL = (CubeMapGL*)mRenderResult->getImpl();
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

void nex::CubeRenderTargetGL::useSide(CubeMap::Side side, unsigned mipLevel)
{
	CubeMapGL* cubeMap = (CubeMapGL*)mRenderResult->getImpl();

	const GLuint AXIS_SIDE = CubeMapGL::translate(side);
	const GLuint cubeMapTexture = cubeMap->getCubeMap();

	bind();

	GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, AXIS_SIDE, cubeMapTexture, mipLevel));
	GLCall(glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}

nex::CubeMapGL * nex::CubeRenderTargetGL::createCopy()
{

	//first create a new cube render target that we use to blit the content
	CubeRenderTargetGL copy(mWidth, mHeight, data);

	GLint readFBId = 0;
	GLint drawFboId = 0;
	GLCall(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId));
	GLCall(glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFBId));


	TextureGL* textureGL = (TextureGL*)mRenderResult->getImpl();
	TextureGL* copyTexGL = (TextureGL*)copy.mRenderResult->getImpl();

	for (int i = 0; i < 6; ++i) {

		//attach the cubemap side of this render target
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, *textureGL->getTexture(), 0));

		//attach the cubemap side of the copy render target
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, copy.mFrameBuffer));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, *copyTexGL->getTexture(), 0));

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// now we can blit the content to the copy
		GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, mFrameBuffer));
		GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, copy.mFrameBuffer));
		GLCall(glBlitFramebuffer(0, 0, mWidth, mHeight,
			0, 0, copy.mWidth, copy.mHeight,
			GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
			GL_NEAREST));
	}

	GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBId));
	GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFboId));

	//extract cubemap texture from the copy and delete copy
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, copy.mFrameBuffer));
	for (int i = 0; i < 6; ++i) {
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0)); // unbound the cubemap side
	}

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	// reset the texture id of the cubemap of the copy, so that it won't be released!
	auto cache = copy.mRenderResult.get();
	copy.mRenderResult = nullptr;

	return (CubeMapGL*)cache;
	
}


void nex::CubeRenderTargetGL::resizeForMipMap(unsigned int mipMapLevel) {

	if (!data.generateMipMaps) {
		throw_with_trace(runtime_error("CubeRenderTargetGL::resizeForMipMap(unsigned int): No mip levels generated for this cube rener target!"));
	}

	assert(mDepthStencilMap != nullptr);

	auto renderBuffer = (RenderBufferGL*)mDepthStencilMap->getImpl();

	const GLuint texture = *renderBuffer->getTexture();
	const GLuint attachment = DepthStencilMapGL::getAttachmentType(renderBuffer->getFormat());

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer));

	unsigned int mipWidth = (unsigned int)(mWidth * std::pow(0.5, mipMapLevel));
	unsigned int mipHeight = (unsigned int)(mHeight * std::pow(0.5, mipMapLevel));
	renderBuffer->resize(mipWidth, mipHeight);
	//GLCall(glBindRenderbuffer(GL_RENDERBUFFER, texture));
	//GLCall(glRenderbufferStorage(GL_RENDERBUFFER, format, mipWidth, mipHeight));
	GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, texture));

	//glBindRenderbuffer(GL_RENDERBUFFER, 0);
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

nex::RenderTarget::RenderTarget(std::unique_ptr<RenderTargetImpl> impl) : mImpl(std::move(impl))
{
}

void nex::RenderTarget::addAttachment(RenderAttachment attachment)
{
	auto gl = (RenderTargetGL*)mImpl.get();
	gl->addAttachment(std::move(attachment));
}

void nex::RenderTarget::bind()
{
	RenderTargetGL* impl = (RenderTargetGL*)mImpl.get();
	impl->bind();
}

void nex::RenderTarget::clear(int components)
{
	int renderComponentsComponentsGL = RenderTarget2DGL::getRenderComponents(components);
	GLCall(glClear(renderComponentsComponentsGL));
}

nex::RenderTargetImpl* nex::RenderTarget::getImpl() const
{
	return mImpl.get();
}

std::shared_ptr<nex::Texture> nex::RenderTarget::getDepthStencilMapShared()
{
	auto gl = (RenderTargetGL*)getImpl();
	return gl->getDepthStencilMapShared();
}

nex::Texture* nex::RenderTarget::getDepthStencilMap()
{
	auto gl = (RenderTargetGL*)getImpl();
	return gl->getDepthStencilMap();
}

nex::Texture* nex::RenderTarget::getRenderResult()
{
	auto gl = (RenderTargetGL*)getImpl();
	return gl->getResult();
}

void nex::RenderTarget::setImpl(std::unique_ptr<RenderTargetImpl> impl)
{
	mImpl = std::move(impl);
}

nex::Texture* nex::RenderTarget::setRenderResult(Texture* texture)
{
	auto gl = (RenderTargetGL*)getImpl();
	return gl->setRenderResult(texture);
}

void nex::RenderTarget::updateAttachments()
{
	auto gl = (RenderTargetGL*)getImpl();
	gl->updateAttachments();
}


void nex::RenderTarget::useDepthStencilMap(std::shared_ptr<Texture> depthStencilMap)
{
	auto gl = (RenderTargetGL*)getImpl();
	gl->useDepthStencilMap(std::move(depthStencilMap));
}

void nex::RenderTarget::unbind()
{
	auto gl = (RenderTargetGL*)getImpl();
	gl->unbind();
}

/*
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
}*/


nex::RenderTargetGL::RenderTargetGL(unsigned width, unsigned height, GLuint frameBuffer, std::shared_ptr<Texture> depthStencilMap) :
	RenderTargetImpl(),
	mFrameBuffer(frameBuffer), mWidth(width), mHeight(height), mRenderResult(make_unique<Texture>(nullptr))
{
}

nex::RenderTargetGL::~RenderTargetGL()
{
	if (mFrameBuffer != GL_FALSE)
	{
		GLCall(glDeleteFramebuffers(1, &mFrameBuffer));
	}

	mFrameBuffer = GL_FALSE;
}

void nex::RenderTargetGL::addAttachment(RenderAttachment attachment)
{
	const GLuint attachmentType = translate(attachment.type, attachment.attachIndex);
	auto gl = (TextureGL*)attachment.texture->getImpl();
	
	GLuint textureID = *gl->getTexture();
	GLuint target = translate(attachment.target);
	GLCall(glFramebufferTexture(GL_FRAMEBUFFER, attachmentType, textureID, 0));

	mAttachments.emplace_back(std::move(attachment));
}

void nex::RenderTargetGL::bind()
{
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer));
}

void nex::RenderTargetGL::unbind()
{
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

nex::Texture* nex::RenderTargetGL::getDepthStencilMap() const
{
	return mDepthStencilMap.get();
}

std::shared_ptr<nex::Texture> nex::RenderTargetGL::getDepthStencilMapShared() const
{
	return mDepthStencilMap;
}

nex::RenderTarget2DGL::RenderTarget2DGL(unsigned width,
	unsigned height,
	const TextureData& data,
	unsigned samples,
	std::shared_ptr<Texture> depthStencilMap) 
:
	RenderTargetGL(width, height, GL_FALSE, std::move(depthStencilMap))
{

	GLuint wrapR = translate(data.wrapR);
	GLuint wrapS = translate(data.wrapS);
	GLuint wrapT = translate(data.wrapT);
	GLuint minFilter = translate(data.minFilter);
	GLuint magFilter = translate(data.magFilter);
	GLuint internalFormat = translate(data.internalFormat);
	GLuint colorspace = translate(data.colorspace);
	GLuint pixelDataType = translate(data.pixelDataType);


	GLuint textureTargetType = GL_TEXTURE_2D;

	const bool isMultiSample = samples > 1;

	if (isMultiSample)
		textureTargetType = GL_TEXTURE_2D_MULTISAMPLE;

	GLCall(glGenFramebuffers(1, &mFrameBuffer));
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer));

	// Generate texture

	GLuint textureID;
	GLCall(glGenTextures(1, &textureID));


	//glActiveTexture(GL_TEXTURE0);
	GLCall(glBindTexture(textureTargetType, textureID));
	//glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);

	if (isMultiSample)
	{
		GLCall(glTexImage2DMultisample(textureTargetType, samples, internalFormat, width, height, GL_TRUE));
	} else
	{
		GLCall(glTexImage2D(textureTargetType, 0, internalFormat, width, height, 0, colorspace, pixelDataType, 0));
	}

	

	GLCall(glTexParameteri(textureTargetType, GL_TEXTURE_MIN_FILTER, minFilter));
	GLCall(glTexParameteri(textureTargetType, GL_TEXTURE_MAG_FILTER, magFilter));

	// clamp is important so that no pixel artifacts occur on the border!
	GLCall(glTexParameteri(textureTargetType, GL_TEXTURE_WRAP_R, wrapR));
	GLCall(glTexParameteri(textureTargetType, GL_TEXTURE_WRAP_S, wrapS));
	GLCall(glTexParameteri(textureTargetType, GL_TEXTURE_WRAP_T, wrapT));

	//swizzle
	if (data.useSwizzle)
	{
		int swizzle[4];
		swizzle[0] = translate(data.swizzle.r);
		swizzle[1] = translate(data.swizzle.g);
		swizzle[2] = translate(data.swizzle.b);
		swizzle[3] = translate(data.swizzle.a);

		GLCall(glTexParameteriv(textureTargetType, GL_TEXTURE_SWIZZLE_RGBA, swizzle));

	}

	//glBindTexture(GL_TEXTURE_2D, 0);

	// attach texture to currently bound frame buffer
	GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textureTargetType, textureID, 0));

	auto resultTexture = getResult();
	resultTexture->setImpl(make_unique<Texture2DGL>(textureID, data, width, height));

	// Set the list of draw buffers.
	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	GLCall(glDrawBuffers(1, DrawBuffers)); // "1" is the size of DrawBuffers



	mDepthStencilMap = depthStencilMap;

	if (mDepthStencilMap != nullptr)
	{
		validateDepthStencilMap(mDepthStencilMap.get());

		const bool isDepthStencil = dynamic_cast<DepthStencilMap*>(depthStencilMap.get()) != nullptr;
		TextureGL* depthStencilMapGL = (TextureGL*)depthStencilMap->getImpl();


		if (isDepthStencil)
		{
			DepthStencilFormat format = dynamic_cast<DepthStencilMap*>(depthStencilMap.get())->getFormat();
			GLuint attachmentType = DepthStencilMapGL::getAttachmentType(format);

			GLCall(glBindTexture(textureTargetType, *depthStencilMapGL->getTexture()));
			GLCall(glFramebufferTexture(GL_FRAMEBUFFER, attachmentType, *depthStencilMapGL->getTexture(), 0));

		}
		else
		{
			DepthStencilFormat format = dynamic_cast<RenderBuffer*>(depthStencilMap.get())->getFormat();
			GLuint attachmentType = DepthStencilMapGL::getAttachmentType(format);

			GLCall(glBindTexture(GL_RENDERBUFFER, *depthStencilMapGL->getTexture()));
			GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachmentType, GL_RENDERBUFFER, *depthStencilMapGL->getTexture()));
		}
	}

	// finally check if all went successfully
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw_with_trace(runtime_error("RenderTarget2DGL::RenderTarget2DGL(): Couldn't successfully init framebuffer!"));
	}

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}


nex::RenderTarget2D::RenderTarget2D(std::unique_ptr<RenderTargetImpl> impl) : RenderTarget(std::move(impl))
{
}

nex::RenderTarget2D::RenderTarget2D(int width, int height, const TextureData& data, unsigned samples,
	std::shared_ptr<Texture> depthStencilMap): RenderTarget(make_unique<RenderTarget2DGL>(width, height, data, samples, std::move(depthStencilMap)))
{
}

void nex::RenderTarget2D::blit(RenderTarget2D * dest, const Dimension & sourceDim, int components)
{
	auto self = (RenderTarget2DGL*)getImpl();
	auto other = (RenderTarget2DGL*)dest->getImpl();
	GLint componentsGL = RenderTarget2DGL::getRenderComponents(components);
	self->blit(other, sourceDim, componentsGL);
}

unsigned nex::RenderTarget2D::getHeight() const
{
	auto gl = (RenderTarget2DGL*)getImpl();
	return gl->getHeight();
}

unsigned nex::RenderTarget2D::getWidth() const
{
	auto gl = (RenderTarget2DGL*)getImpl();
	return gl->getWidth();
}


void nex::RenderTarget2DGL::blit(RenderTarget2DGL* dest, const Dimension& sourceDim, GLuint components)
{

	GLint readFBId = 0;
	GLint drawFboId = 0;
	GLCall(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId));
	GLCall(glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFBId));

	GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, dest->getFrameBuffer()));
	GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, getFrameBuffer()));
	GLCall(glBlitFramebuffer(sourceDim.xPos, sourceDim.yPos, sourceDim.width, sourceDim.height,
		sourceDim.xPos, sourceDim.yPos, sourceDim.width, sourceDim.height,
		components,
		GL_NEAREST));
	GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBId));
	GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFboId));
}

GLint nex::RenderTarget2DGL::getRenderComponents(int components)
{
	int componentsGL = 0;
	if (components & RenderComponent::Color) componentsGL |= GL_COLOR_BUFFER_BIT;
	if (components & RenderComponent::Depth) componentsGL |= GL_DEPTH_BUFFER_BIT;
	if (components & RenderComponent::Stencil) componentsGL |= GL_STENCIL_BUFFER_BIT;

	return componentsGL;
}

void nex::RenderTargetGL::validateDepthStencilMap(Texture* texture)
{
	const bool isDepthStencil = dynamic_cast<DepthStencilMap*>(texture) != nullptr;
	const bool isRenderBuffer = dynamic_cast<RenderBuffer*>(texture) != nullptr;

	if (!isDepthStencil && !isRenderBuffer)
		throw std::runtime_error("nex::RenderTargetGL::validateDepthStencilMap failed: Wrong texture input!");
}

nex::RenderTarget2DGL::RenderTarget2DGL(unsigned width, unsigned height) : RenderTargetGL(width, height, GL_FALSE, nullptr)
{
}

GLuint nex::RenderTargetGL::getFrameBuffer() const
{
	return mFrameBuffer;
}

nex::Texture* nex::RenderTargetGL::getResult() const
{
	return mRenderResult.get();
}

unsigned nex::RenderTargetGL::getWidth() const
{
	return mWidth;
}

unsigned nex::RenderTargetGL::getHeight() const
{
	return mHeight;
}


void nex::RenderTargetGL::setFrameBuffer(GLuint newValue)
{
	mFrameBuffer = newValue;
}

nex::Texture* nex::RenderTargetGL::setRenderResult(Texture* texture)
{
	const auto old = mRenderResult.release();
	mRenderResult.reset(texture);
	return old;
}

void nex::RenderTargetGL::useDepthStencilMap(std::shared_ptr<Texture> depthStencilMap)
{

	auto renderBuffer = dynamic_cast<RenderBufferGL*> (depthStencilMap->getImpl());
	auto depthStencilMapGL = dynamic_cast<DepthStencilMapGL*> (depthStencilMap->getImpl());

	if (renderBuffer)
	{
		const GLuint texture = *renderBuffer->getTexture();
		const GLuint attachment = DepthStencilMapGL::getAttachmentType(renderBuffer->getFormat());
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer));
		GLCall(glBindRenderbuffer(GL_RENDERBUFFER, texture)); //TODO
		GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, texture));
	} else if (depthStencilMapGL)
	{
		const GLuint texture = *depthStencilMapGL->getTexture();
		const auto& desc = depthStencilMapGL->getDescription();
		const GLuint attachment = DepthStencilMapGL::getAttachmentType(desc.format);
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer));
		GLCall(glBindTexture(GL_TEXTURE_2D, texture)); //TODO
		GLCall(glFramebufferTexture(GL_FRAMEBUFFER, attachment, texture, 0));
	}

	mDepthStencilMap = std::move(depthStencilMap);
}

void nex::RenderTargetGL::updateAttachments()
{
	std::vector<GLuint> drawBufferList(mAttachments.size());
	for (unsigned i = 0; i < drawBufferList.size(); ++i)
	{
		const auto& attachment = mAttachments[i];
		drawBufferList[i] = translate(attachment.type, attachment.attachIndex);
	}

	GLCall(glDrawBuffers(drawBufferList.size(), drawBufferList.data()));
}


nex::CubeDepthMap* nex::CubeDepthMap::create(unsigned width, unsigned height)
{
	return new CubeDepthMap(width, height);
}

nex::CubeDepthMap::CubeDepthMap(int width, int height) : RenderTarget(make_unique<CubeDepthMapGL>(width, height))
{
}

nex::CubeDepthMapGL::CubeDepthMapGL(int width, int height) :
	RenderTargetGL(width, height)
{

	mRenderResult = make_unique<CubeMap>(width, height);

	GLuint texture;
	GLCall(glGenTextures(1, &texture));
	glGenFramebuffers(1, &mFrameBuffer);
	auto textureGL = (TextureGL*)mRenderResult->getImpl();
	textureGL->setTexture(texture);

	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	for (int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_COMPONENT, GL_RENDERBUFFER, texture);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw_with_trace(runtime_error("CubeDepthMapGL::CubeDepthMapGL(int, int): Framebuffer not complete!"));

	// A depth map only needs depth (z-value) informations; therefore disable any color buffers
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	((CubeMapGL*)mRenderResult->getImpl())->setCubeMap(texture);
}


nex::PBR_GBuffer* nex::PBR_GBuffer::create(unsigned width, unsigned height)
{
	return new PBR_GBuffer(width, height);
}

nex::PBR_GBuffer::PBR_GBuffer(int width, int height) : RenderTarget(make_unique<PBR_GBufferGL>(width, height))
{
}

nex::Texture* nex::PBR_GBuffer::getAlbedo() const
{
	auto gl = (PBR_GBufferGL*)getImpl();
	return gl->getAlbedo();
}

nex::Texture* nex::PBR_GBuffer::getAoMetalRoughness() const
{
	auto gl = (PBR_GBufferGL*)getImpl();
	return gl->getAoMetalRoughness();
}

nex::Texture* nex::PBR_GBuffer::getNormal() const
{
	auto gl = (PBR_GBufferGL*)getImpl();
	return gl->getNormal();
}

nex::Texture* nex::PBR_GBuffer::getDepth() const
{
	auto gl = (PBR_GBufferGL*)getImpl();
	return gl->getDepth();
}

nex::PBR_GBufferGL::PBR_GBufferGL(int width, int height)
	: 
	RenderTargetGL(width, height)
{
	GLCall(glGenFramebuffers(1, &mFrameBuffer));
	glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
	//unsigned int gPosition, gNormal, gAlbedo;
	unsigned int tempTexture;

	TextureData data;
	data.minFilter = TextureFilter::NearestNeighbor;
	data.magFilter = TextureFilter::NearestNeighbor;
	data.wrapR = TextureUVTechnique::ClampToEdge;
	data.wrapS = TextureUVTechnique::ClampToEdge;
	data.wrapT = TextureUVTechnique::ClampToEdge;
	data.generateMipMaps = false;
	data.useSwizzle = false;



	// albedo
	data.colorspace = ColorSpace::RGB;
	data.pixelDataType = PixelDataType::FLOAT;
	data.internalFormat = InternFormat::RGB16F;
	albedo.texture = make_shared<Texture2D>(width, height, data, nullptr);
	albedo.attachIndex = 0;
	addAttachment(albedo);

	//make_unique<Texture2D>(width, height, TextureData(), nullptr)
	
	/*glGenTextures(1, &tempTexture);
	((TextureGL*)albedo->getImpl())->setTexture(tempTexture);

	glBindTexture(GL_TEXTURE_2D, tempTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tempTexture, 0);*/

	// ao metal roughness
	data.internalFormat = InternFormat::RGB8;
	data.pixelDataType = PixelDataType::UBYTE;
	aoMetalRoughness.texture = make_shared<Texture2D>(width, height, data, nullptr);
	aoMetalRoughness.attachIndex = 1;
	addAttachment(aoMetalRoughness);

	/*glGenTextures(1, &tempTexture);
	((TextureGL*)aoMetalRoughness->getImpl())->setTexture(tempTexture);

	glBindTexture(GL_TEXTURE_2D, tempTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tempTexture, 0);*/

	// normal
	data.internalFormat = InternFormat::RGB16F;
	data.pixelDataType = PixelDataType::FLOAT;
	normal.texture = make_shared<Texture2D>(width, height, data, nullptr);
	normal.attachIndex = 2;
	addAttachment(normal);


	/*glGenTextures(1, &tempTexture);
	((TextureGL*)normal->getImpl())->setTexture(tempTexture);

	glBindTexture(GL_TEXTURE_2D, tempTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, tempTexture, 0);*/

	// position
	/*glGenTextures(1, &tempTexture);
	((TextureGL*)position->getImpl())->setTexture(tempTexture);

	glBindTexture(GL_TEXTURE_2D, tempTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, tempTexture, 0);*/

	// depth
	data.internalFormat = InternFormat::R32F;
	data.pixelDataType = PixelDataType::FLOAT;
	depth.texture = make_shared<Texture2D>(width, height, data, nullptr);
	depth.attachIndex = 3;
	addAttachment(depth);



	/*glGenTextures(1, &tempTexture);
	((TextureGL*)depth->getImpl())->setTexture(tempTexture);

	glBindTexture(GL_TEXTURE_2D, tempTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, tempTexture, 0);*/



	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	/*unsigned int attachments[4] = { translate(albedo.type, albedo.attachIndex), 
		translate(aoMetalRoughness.type, aoMetalRoughness.attachIndex),
		translate(normal.type, normal.attachIndex),
		translate(depth.type, depth.attachIndex)
	};

	glDrawBuffers(4, attachments);*/
	updateAttachments();

	// create and attach depth buffer (renderbuffer)
	/*unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	depth.setTexture(rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepth);*/

	// depth/stencil
	DepthStencilDesc desc;
	desc.minFilter = TextureFilter::NearestNeighbor;
	desc.magFilter = TextureFilter::NearestNeighbor;
	desc.wrap = TextureUVTechnique::ClampToEdge;
	desc.format = DepthStencilFormat::DEPTH24_STENCIL8;
	auto depthBuffer = make_shared<DepthStencilMap>(width, height, desc);

	useDepthStencilMap(std::move(depthBuffer));

	//auto renderBuffer = make_unique<RenderBuffer>(width, height, DepthStencilFormat::DEPTH24_STENCIL8);

	//useDepthStencilMap(renderBuffer.get());
	//renderBuffer.release();


	/*glGenTextures(1, &renderBuffer);
	((RenderBufferGL*)depth->getImpl())->setTexture(renderBuffer);

	glBindTexture(GL_TEXTURE_2D, renderBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);*/
	
	
	//glFramebufferTexture(GL_FRAMEBUFFER, DepthStencilMapGL::getAttachmentType(desc.format), *depthGL->getTexture(), 0);

	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw_with_trace(std::runtime_error("PBR_DeferredGL::createMultipleRenderTarget(int, int): Couldn't successfully init framebuffer!"));
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	
}

nex::Texture2D* nex::PBR_GBufferGL::getAlbedo() const
{
	return (Texture2D*)albedo.texture.get();
}

nex::Texture2D * nex::PBR_GBufferGL::getAoMetalRoughness() const
{
	return (Texture2D*)aoMetalRoughness.texture.get();
}

nex::Texture2D * nex::PBR_GBufferGL::getNormal() const
{
	return (Texture2D*)normal.texture.get();
}

nex::Texture2D* nex::PBR_GBufferGL::getDepth() const
{
	return (Texture2D*)depth.texture.get();
}