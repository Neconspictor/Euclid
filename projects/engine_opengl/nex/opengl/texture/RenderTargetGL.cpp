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
	RenderTargetGL(), data(data), mWidth(width), mHeight(height)
{	
	bind();

	RenderAttachment color;
	color.target = TextureTarget::CUBE_MAP;
	color.type = RenderAttachment::Type::COLOR;
	color.texture = make_unique<CubeMap>(width, height, data);
	addAttachment(std::move(color));


	//TODO generify!
	RenderAttachment depth;
	depth.target = TextureTarget::TEXTURE2D;
	depth.type = RenderAttachment::Type::DEPTH;
	depth.texture = make_unique<RenderBuffer>(width, height, InternFormat::DEPTH24);
	addAttachment(std::move(depth));

	finalizeAttachments();
}

void nex::CubeRenderTargetGL::useSide(CubeMap::Side side, unsigned mipLevel)
{
	//TODO
	//bind();

	const unsigned index = 0;

	auto& attachment = mAttachments[index];
	attachment.mipmapLevel = mipLevel;
	attachment.side = side;
	updateAttachment(index);
	
	//TODO
	//GLCall(glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}

/*nex::CubeMapGL * nex::CubeRenderTargetGL::createCopy()
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
	
}*/


void nex::CubeRenderTargetGL::resizeForMipMap(unsigned int mipMapLevel) {

	if (!data.generateMipMaps) {
		throw_with_trace(runtime_error("CubeRenderTargetGL::resizeForMipMap(unsigned int): No mip levels generated for this cube render target!"));
	}

	const unsigned renderBufferIndex = 1;

	auto& renderBufferAttachment = mAttachments[renderBufferIndex];

	auto renderBuffer = (RenderBufferGL*)renderBufferAttachment.texture->getImpl();

	bind();

	unsigned int mipWidth = (unsigned int)(mWidth * std::pow(0.5, mipMapLevel));
	unsigned int mipHeight = (unsigned int)(mHeight * std::pow(0.5, mipMapLevel));
	renderBuffer->resize(mipWidth, mipHeight);

	updateAttachment(renderBufferIndex);

	//TODO validate!
	//GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, texture));
	//unbind();
}

nex::RenderTarget::RenderTarget(std::unique_ptr<RenderTargetImpl> impl) : mImpl(std::move(impl))
{
}

nex::RenderTarget::RenderTarget() : mImpl(make_unique<RenderTargetGL>())
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

void nex::RenderTarget::finalizeAttachments()
{
	auto gl = (RenderTargetGL*)getImpl();
	gl->finalizeAttachments();
}

nex::RenderTargetImpl* nex::RenderTarget::getImpl() const
{
	return mImpl.get();
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

void nex::RenderTarget::unbind()
{
	auto gl = (RenderTargetGL*)getImpl();
	gl->unbind();
}

void nex::RenderTarget::updateAttachment(unsigned index)
{
	auto gl = (RenderTargetGL*)getImpl();
	gl->updateAttachment(index);
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

nex::RenderTargetGL::RenderTargetGL() :
	RenderTargetImpl(),
	mFrameBuffer(GL_FALSE)
{
		GLCall(glGenFramebuffers(1, &mFrameBuffer));
}


nex::RenderTargetGL::RenderTargetGL(GLuint frameBuffer) :
	RenderTargetImpl(),
	mFrameBuffer(frameBuffer)
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
	mAttachments.emplace_back(std::move(attachment));
	updateAttachment(mAttachments.size() - 1);
}

void nex::RenderTargetGL::bind()
{
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer));
}

void nex::RenderTargetGL::unbind()
{
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void nex::RenderTargetGL::updateAttachment(unsigned index)
{
	const auto& attachment = mAttachments[index];

	const auto gl = (TextureGL*)attachment.texture->getImpl();
	const GLuint textureID = *gl->getTexture();


	auto renderBuffer = dynamic_cast<RenderBuffer*> (attachment.texture->getImpl());

	const GLuint attachmentType = translate(attachment.type, attachment.attachIndex);

	auto layer = attachment.layer;

	if (attachment.target == TextureTarget::CUBE_MAP) {
		layer = getLayerFromCubeMapSide(attachment.side);
	}

	if (renderBuffer)
	{
		GLCall(glNamedFramebufferRenderbuffer(mFrameBuffer, attachmentType, GL_RENDERBUFFER, textureID));
	} else
	{
		//NOTE: OpenGL 4.5 or DSA extension is needed for textures not being array, cube or 3d
		GLCall(glNamedFramebufferTextureLayer(mFrameBuffer, attachmentType, textureID, attachment.mipmapLevel, layer));
	}
}

nex::RenderTarget2DGL::RenderTarget2DGL(unsigned width,
	unsigned height,
	const TextureData& data,
	unsigned samples) 
:
	RenderTargetGL()
{

	const bool isMultiSample = samples > 1;

	bind();

	RenderAttachment color;

	if (isMultiSample)
	{
		color.target = TextureTarget::TEXTURE2D_MULTISAMPLE;
		color.texture = make_unique<Texture2DMultisample>(width, height, data, samples);
	} else
	{
		color.target = TextureTarget::TEXTURE2D;
		color.texture = make_unique<Texture2D>(width, height, data, nullptr);
	}

	addAttachment(std::move(color));

	//TODO handle depth-stencil map!


	/*if (depthStencilMap != nullptr)
	{
		RenderAttachment depth;
		depth.texture = std::move(depthStencilMap);
		depth.target = TextureTarget::TEXTURE2D;
		depth.type = RenderAttachment::Type::DEPTH;
		// Note: target and type haven't to be set for depth-/stencil textures
		addAttachment(std::move(depth));
	}*/

	
	finalizeAttachments();


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

nex::RenderTarget2D::RenderTarget2D(int width, int height, const TextureData& data, unsigned samples): RenderTarget(make_unique<RenderTarget2DGL>(width, height, data, samples))
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

GLuint nex::RenderTargetGL::getFrameBuffer() const
{
	return mFrameBuffer;
}

unsigned nex::RenderTargetGL::getLayerFromCubeMapSide(CubeMap::Side side)
{
	return (unsigned)side;
}

bool nex::RenderTargetGL::isComplete() const
{
	bool result;
	GLCall(result = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	return result;
}


void nex::RenderTargetGL::setFrameBuffer(GLuint newValue)
{
	mFrameBuffer = newValue;
}

void nex::RenderTargetGL::finalizeAttachments()
{
	std::vector<GLuint> drawBufferList(mAttachments.size());
	for (unsigned i = 0; i < drawBufferList.size(); ++i)
	{
		const auto& attachment = mAttachments[i];
		drawBufferList[i] = translate(attachment.type, attachment.attachIndex);
	}

	GLCall(glDrawBuffers(drawBufferList.size(), drawBufferList.data()));
}

const std::vector<nex::RenderAttachment>& nex::RenderTargetGL::getAttachments()
{
	return mAttachments;
}


nex::CubeDepthMap* nex::CubeDepthMap::create(unsigned width, unsigned height)
{
	return new CubeDepthMap(width, height);
}

nex::CubeDepthMap::CubeDepthMap(int width, int height) : RenderTarget(make_unique<CubeDepthMapGL>(width, height))
{
}

nex::CubeDepthMapGL::CubeDepthMapGL(int width, int height) :
	RenderTargetGL()
{

	TextureData desc;
	desc.minFilter = TextureFilter::NearestNeighbor;
	desc.magFilter = TextureFilter::NearestNeighbor;
	desc.wrapS = desc.wrapR = desc.wrapT = TextureUVTechnique::ClampToEdge;
	desc.pixelDataType = PixelDataType::FLOAT;
	desc.colorspace = ColorSpace::DEPTH;
	desc.internalFormat = InternFormat::DEPTH_COMPONENT32F;

	RenderAttachment depth;

	depth.texture = make_unique<CubeMap>(width, height, desc);
	depth.type = RenderAttachment::Type::DEPTH;
	depth.target = TextureTarget::CUBE_MAP;
	depth.side = CubeMap::Side::POSITIVE_X;

	bind();
	addAttachment(std::move(depth));
	finalizeAttachments();

	// TODO validate!
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);

	if (!isComplete())
		throw_with_trace(runtime_error("CubeDepthMapGL::CubeDepthMapGL(int, int): Framebuffer not complete!"));

	// A depth map only needs depth (z-value) informations; therefore disable any color buffers
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	unbind();
}