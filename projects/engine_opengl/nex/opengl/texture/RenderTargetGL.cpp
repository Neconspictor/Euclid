#include <nex/opengl/texture/RenderTargetGL.hpp>
#include <cassert>
#include <nex/util/ExceptionHandling.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <nex/opengl/opengl.hpp>
#include <nex/RenderBackend.hpp>

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

void nex::CubeRenderTarget::resizeForMipMap(unsigned mipMapLevel)
{
	CubeRenderTargetGL* gl = (CubeRenderTargetGL*)getImpl();
	gl->resizeForMipMap(mipMapLevel);
}


nex::CubeRenderTargetGL::CubeRenderTargetGL(unsigned width, unsigned height, TextureData data) :
	RenderTargetGL(width, height, nullptr), data(data)
{
	mRenderResult = make_unique<CubeMap>(width, height);

	auto renderBuffer = make_shared<RenderBuffer>(width, height, DepthStencilFormat::DEPTH24);
	
	bind();
	useDepthStencilMap(renderBuffer);

	GLuint internalFormat = translate(data.internalFormat);
	GLuint colorspace = translate(data.colorspace);
	GLuint pixelDataType = translate(data.pixelDataType);


	//pre-allocate the six faces of the cubemap
	CubeMapGL* textureGL = (CubeMapGL*)mRenderResult->getImpl();

	TextureGL::generateTexture(textureGL->getTexture(), data, GL_TEXTURE_CUBE_MAP);

	GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, *textureGL->getTexture()));
	for (int i = 0; i < 6; ++i)
	{
		GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, colorspace,
			pixelDataType, nullptr));
	}

	if (data.generateMipMaps)
	{
		GLCall(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));
	}

	unbind();
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
	copy.bind();
	for (int i = 0; i < 6; ++i) {
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0)); // unbound the cubemap side
	}

	copy.unbind();

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

	bind();

	unsigned int mipWidth = (unsigned int)(mWidth * std::pow(0.5, mipMapLevel));
	unsigned int mipHeight = (unsigned int)(mHeight * std::pow(0.5, mipMapLevel));
	renderBuffer->resize(mipWidth, mipHeight);
	//GLCall(glBindRenderbuffer(GL_RENDERBUFFER, texture));
	//GLCall(glRenderbufferStorage(GL_RENDERBUFFER, format, mipWidth, mipHeight));
	GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, texture));

	unbind();
}

nex::RenderTarget::RenderTarget(std::unique_ptr<RenderTargetImpl> impl) : mImpl(std::move(impl))
{
}

nex::RenderTarget::RenderTarget(unsigned width, unsigned height) : mImpl(make_unique<RenderTargetGL>(width, height, nullptr))
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

unsigned nex::RenderTarget::getHeight() const
{
	auto gl = (RenderTargetGL*)getImpl();
	return gl->getHeight();
}

unsigned nex::RenderTarget::getWidth() const
{
	auto gl = (RenderTargetGL*)getImpl();
	return gl->getWidth();
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

bool nex::RenderTarget::isComplete() const
{
	auto gl = (RenderTargetGL*)getImpl();
	return gl->isComplete();
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

nex::RenderTargetGL::RenderTargetGL(unsigned width, unsigned height, std::shared_ptr<Texture> depthStencilMap) :
	RenderTargetImpl(),
	mFrameBuffer(GL_FALSE), 
	mWidth(width), 
	mHeight(height), 
	mRenderResult(make_unique<Texture>(nullptr)), 
	mDepthStencilMap(std::move(depthStencilMap))
{
		GLCall(glGenFramebuffers(1, &mFrameBuffer));
}


nex::RenderTargetGL::RenderTargetGL(unsigned width, unsigned height, GLuint frameBuffer, std::shared_ptr<Texture> depthStencilMap) :
	RenderTargetImpl(),
	mFrameBuffer(frameBuffer), 
	mWidth(width), 
	mHeight(height), 
	mRenderResult(make_unique<Texture>(nullptr)), 
	mDepthStencilMap(std::move(depthStencilMap))
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

nex::RenderTarget2DGL::RenderTarget2DGL(unsigned width, unsigned height, GLuint frameBuffer, std::shared_ptr<Texture> depthStencilMap) 
: RenderTargetGL(width, height, frameBuffer, std::move(depthStencilMap))
{
}

nex::RenderTarget2DGL::RenderTarget2DGL(unsigned width,
	unsigned height,
	const TextureData& data,
	unsigned samples,
	std::shared_ptr<Texture> depthStencilMap) 
:
	RenderTargetGL(width, height, std::move(depthStencilMap))
{
	GLuint internalFormat = translate(data.internalFormat);
	GLuint colorspace = translate(data.colorspace);
	GLuint pixelDataType = translate(data.pixelDataType);


	GLuint textureTargetType = GL_TEXTURE_2D;

	const bool isMultiSample = samples > 1;

	if (isMultiSample)
		textureTargetType = GL_TEXTURE_2D_MULTISAMPLE;

	bind();

	// Generate texture

	GLuint textureID;
	TextureGL::generateTexture(&textureID, data, textureTargetType);

	GLCall(glBindTexture(textureTargetType, textureID));


	if (isMultiSample)
	{
		GLCall(glTexImage2DMultisample(textureTargetType, samples, internalFormat, width, height, GL_TRUE));
	} else
	{
		GLCall(glTexImage2D(textureTargetType, 0, internalFormat, width, height, 0, colorspace, pixelDataType, 0));
	}

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

	unbind();
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

bool nex::RenderTargetGL::isComplete() const
{
	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
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
		bind();
		GLCall(glBindRenderbuffer(GL_RENDERBUFFER, texture)); //TODO
		GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, texture));
	} else if (depthStencilMapGL)
	{
		const GLuint texture = *depthStencilMapGL->getTexture();
		const auto& desc = depthStencilMapGL->getDescription();
		const GLuint attachment = DepthStencilMapGL::getAttachmentType(desc.format);
		bind();
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
	RenderTargetGL(width, height, nullptr)
{

	mRenderResult = make_unique<CubeMap>(width, height);

	GLuint texture;
	BaseTextureDesc desc;
	desc.minFilter = TextureFilter::NearestNeighbor;
	desc.magFilter = TextureFilter::NearestNeighbor;
	desc.wrapS = desc.wrapR = desc.wrapT = TextureUVTechnique::ClampToEdge;
	TextureGL::generateTexture(&texture, desc, GL_TEXTURE_CUBE_MAP);


	auto textureGL = (TextureGL*)mRenderResult->getImpl();
	textureGL->setTexture(texture);

	bind();

	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	for (int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	}

	bind();
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_COMPONENT, GL_RENDERBUFFER, texture);

	if (!isComplete())
		throw_with_trace(runtime_error("CubeDepthMapGL::CubeDepthMapGL(int, int): Framebuffer not complete!"));

	// A depth map only needs depth (z-value) informations; therefore disable any color buffers
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	unbind();

	((CubeMapGL*)mRenderResult->getImpl())->setCubeMap(texture);
}