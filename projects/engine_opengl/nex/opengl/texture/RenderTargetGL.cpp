#include <nex/opengl/texture/RenderTargetGL.hpp>
#include <cassert>
#include <nex/util/ExceptionHandling.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <nex/RenderBackend.hpp>
#include <nex/opengl/texture/TextureGL.hpp>
#include <nex/opengl/opengl.hpp>

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

void nex::CubeRenderTarget::useSide(CubeMapSide side, unsigned mipLevel)
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

unsigned nex::CubeRenderTarget::getWidth() const
{
	return ((CubeRenderTargetGL*)getImpl())->getWidth();
}

unsigned nex::CubeRenderTarget::getHeight() const
{
	return ((CubeRenderTargetGL*)getImpl())->getHeight();
}


nex::CubeRenderTargetGL::CubeRenderTargetGL(unsigned width, unsigned height, TextureData data, InternFormat depthFormat) :
	RenderTargetGL(), data(data), mWidth(width), mHeight(height)
{	
	bind();

	RenderAttachment color;
	color.target = TextureTarget::CUBE_MAP;
	color.type = RenderAttachment::Type::COLOR;
	color.texture = make_unique<CubeMap>(width, height, data);
	addColorAttachment(std::move(color));

	finalizeColorAttachments();


	//TODO generify!
	/*RenderAttachment depth;
	depth.target = TextureTarget::TEXTURE2D;
	depth.type = RenderAttachment::Type::DEPTH;
	depth.texture = make_unique<RenderBuffer>(width, height, depthFormat);
	useDepthAttachment(std::move(depth));*/
}

void nex::CubeRenderTargetGL::useSide(CubeMapSide side, unsigned mipLevel)
{
	//TODO
	//bind();

	const unsigned index = 0;

	auto& attachment = mColorAttachments[index];
	attachment.mipmapLevel = mipLevel;
	attachment.side = side;
	updateColorAttachment(index);
	
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

	auto renderBuffer = dynamic_cast<RenderBufferGL*>(mDepthAttachment.texture->getImpl());

	bind();

	unsigned int mipWidth = (unsigned int)(mWidth * std::pow(0.5, mipMapLevel));
	unsigned int mipHeight = (unsigned int)(mHeight * std::pow(0.5, mipMapLevel));
	renderBuffer->resize(mipWidth, mipHeight);

	updateDepthAttachment();

	//TODO validate!
	//GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, texture));
	//unbind();
}

nex::RenderAttachment::Type nex::RenderAttachment::translate(InternFormat format)
{
	static nex::RenderAttachment::Type const table[]
	{
		Type::COLOR,
		Type::COLOR,
		Type::COLOR,
		Type::COLOR,
		Type::COLOR,
		Type::COLOR,

		Type::COLOR,
		Type::COLOR,
		Type::COLOR,
		Type::COLOR,
		Type::COLOR,
		Type::COLOR,

		Type::COLOR,
		Type::COLOR,
		Type::COLOR,
		Type::COLOR,
		Type::COLOR,
		Type::COLOR,

		Type::COLOR,
		Type::COLOR,
		Type::COLOR,
		Type::COLOR,
		Type::COLOR,
		Type::COLOR,
		Type::COLOR,

		Type::COLOR,
		Type::COLOR,

		Type::DEPTH_STENCIL,
		Type::DEPTH_STENCIL,

		Type::DEPTH,
		Type::DEPTH,
		Type::DEPTH,
		Type::DEPTH,

		Type::STENCIL
	};

	static const unsigned size = (unsigned)InternFormat::LAST - (unsigned)InternFormat::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: InternFormat and nex::RenderAttachment::Type table don't match!");

	return table[(unsigned)format];
}

nex::RenderTarget::RenderTarget(std::unique_ptr<RenderTargetImpl> impl) : mImpl(std::move(impl))
{
}

nex::RenderTarget::RenderTarget() : mImpl(make_unique<RenderTargetGL>())
{
}

void nex::RenderTarget::addColorAttachment(RenderAttachment attachment)
{
	auto gl = (RenderTargetGL*)mImpl.get();
	gl->addColorAttachment(std::move(attachment));
}

void nex::RenderTarget::assertCompletion() const
{
	auto gl = (RenderTargetGL*)mImpl.get();
	gl->assertCompletion();
}

void nex::RenderTarget::bind()
{
	RenderTargetGL* impl = (RenderTargetGL*)mImpl.get();
	impl->bind();
}

void nex::RenderTarget::clear(int components) const
{
	int renderComponentsComponentsGL = RenderTarget2DGL::getRenderComponents(components);
	GLCall(glClear(renderComponentsComponentsGL));
}

void nex::RenderTarget::enableDrawToColorAttachments(bool enable) const
{
	auto gl = (RenderTargetGL*)mImpl.get();
	gl->enableDrawToColorAttachments(enable);
}

void nex::RenderTarget::enableReadFromColorAttachments(bool enable) const
{
	auto gl = (RenderTargetGL*)mImpl.get();
	gl->enableReadFromColorAttachments(enable);
}

void nex::RenderTarget::finalizeAttachments() const
{
	auto gl = (RenderTargetGL*)getImpl();
	gl->finalizeColorAttachments();
}

std::vector<nex::RenderAttachment>& nex::RenderTarget::getColorAttachments()
{
	auto gl = (RenderTargetGL*)getImpl();
	return gl->getColorAttachments();
}

nex::RenderAttachment* nex::RenderTarget::getDepthAttachment()
{
	auto gl = (RenderTargetGL*)getImpl();
	return &gl->getDepthAttachment();
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

void nex::RenderTarget::unbind() const
{
	auto gl = (RenderTargetGL*)getImpl();
	gl->unbind();
}

void nex::RenderTarget::updateColorAttachment(unsigned index) const
{
	auto gl = (RenderTargetGL*)getImpl();
	gl->updateColorAttachment(index);
}

void nex::RenderTarget::updateDepthAttachment() const
{
	auto gl = (RenderTargetGL*)getImpl();
	gl->updateDepthAttachment();
}

void nex::RenderTarget::useDepthAttachment(RenderAttachment attachment) const
{
	auto gl = (RenderTargetGL*)getImpl();
	gl->useDepthAttachment(std::move(attachment));
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

void nex::RenderTargetGL::addColorAttachment(RenderAttachment attachment)
{
	if (attachment.type != RenderAttachment::Type::COLOR)
	{
		throw_with_trace(std::runtime_error("nex::RenderTargetGL::addColorAttachment(): attachment isn't a color component!"));
	}

	mColorAttachments.emplace_back(std::move(attachment));
	updateColorAttachment(mColorAttachments.size() - 1);
}

void nex::RenderTargetGL::assertCompletion() const
{
	if (!isComplete())
	{
		throw_with_trace(std::runtime_error("RenderTargetGL::assertCompletion(): RenderTargetGL not complete!"));
	}
}

void nex::RenderTargetGL::bind() const
{
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer));
}

void nex::RenderTargetGL::enableDrawToColorAttachments(bool enable) const
{
	if (enable)
	{
		const auto colorAttachments = calcColorAttachments();
		GLCall(glNamedFramebufferDrawBuffers(mFrameBuffer, colorAttachments.size(), colorAttachments.data()));
	}
	else
	{
		GLCall(glNamedFramebufferDrawBuffer(mFrameBuffer, GL_NONE));
	}
}

void nex::RenderTargetGL::enableReadFromColorAttachments(bool enable) const
{
	if (enable)
	{
		const auto colorAttachments = calcColorAttachments();

		GLenum buf = 0;

		for (auto buffer : colorAttachments)
		{
			buf |= buffer;
		}

		GLCall(glNamedFramebufferReadBuffer(mFrameBuffer, buf));
	}
	else
	{
		GLCall(glNamedFramebufferReadBuffer(mFrameBuffer, GL_NONE));
	}
}

void nex::RenderTargetGL::unbind() const
{
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void nex::RenderTargetGL::updateColorAttachment(unsigned index) const
{
	const auto& attachment = mColorAttachments[index];
	updateAttachment(attachment);
}

void nex::RenderTargetGL::updateDepthAttachment() const
{
	updateAttachment(mDepthAttachment);
}

const nex::RenderAttachment* nex::RenderTargetGL::getByIndex(const unsigned colorAttachIndex) const
{
	for (auto& attachment : mColorAttachments)
	{
		if (attachment.colorAttachIndex == colorAttachIndex)
			return &attachment;
	}

	return nullptr;
}

void nex::RenderTargetGL::updateAttachment(const RenderAttachment& attachment) const
{
	if (attachment.type == RenderAttachment::Type::COLOR)
	{
		// check that there is no other color attachment with the same color attach index
		const auto* firstFound = getByIndex(attachment.colorAttachIndex);

		if (firstFound != &attachment)
		{
			throw_with_trace(std::runtime_error("nex::RenderTargetGL::updateAttachment(): colorAttachIndex is already used by another attachment!"));
		}
	}

	GLuint textureID = 0; // zero for detaching any currently bound texture

	if (attachment.texture != nullptr)
	{
		const auto gl = (TextureGL*)attachment.texture->getImpl();
		textureID = *gl->getTexture();
	}

	const auto renderBuffer = dynamic_cast<RenderBuffer*> (attachment.texture.get());

	const GLuint attachmentType = translate(attachment.type, attachment.colorAttachIndex);

	auto layer = attachment.layer;

	if (attachment.target == TextureTarget::CUBE_MAP) {
		layer = getLayerFromCubeMapSide(attachment.side);
	}

	if (renderBuffer)
	{
		GLCall(glNamedFramebufferRenderbuffer(mFrameBuffer, attachmentType, GL_RENDERBUFFER, textureID));
	}
	else
	{
		//NOTE: OpenGL 4.5 or DSA extension is needed for textures not being array, cube or 3d
		GLCall(glNamedFramebufferTextureLayer(mFrameBuffer, attachmentType, textureID, attachment.mipmapLevel, layer));
	}
}

std::vector<GLenum> nex::RenderTargetGL::calcColorAttachments() const
{
	std::vector<GLenum> result;
	for (const auto& attachment : mColorAttachments)
	{
		if (attachment.type == RenderAttachment::Type::COLOR)
		{
			const auto glEnum = translate(attachment.type, attachment.colorAttachIndex);
			result.push_back(glEnum);
		}
	}

	return result;
}

nex::RenderTarget2DGL::RenderTarget2DGL(unsigned width,
	unsigned height,
	const TextureData& data,
	unsigned samples) 
:
	RenderTargetGL(),
mHeight(height),
mWidth(width)
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

	addColorAttachment(std::move(color));

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

	
	finalizeColorAttachments();
	assertCompletion();

	unbind();
}

nex::RenderTarget2DGL::RenderTarget2DGL(GLuint frameBuffer, unsigned width, unsigned height) : 
RenderTargetGL(frameBuffer),
mHeight(height),
mWidth(width)
{
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

unsigned nex::RenderTarget2D::getWidth() const
{
	return ((RenderTarget2DGL*)getImpl())->getWidth();
}

unsigned nex::RenderTarget2D::getHeight() const
{
	return ((RenderTarget2DGL*)getImpl())->getHeight();
}


void nex::RenderTarget2DGL::blit(RenderTarget2DGL* dest, const Dimension& sourceDim, GLuint components) const
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
unsigned nex::RenderTarget2DGL::getWidth() const
{
	return mWidth;
}

unsigned nex::RenderTarget2DGL::getHeight() const
{
	return mHeight;
}

GLuint nex::RenderTargetGL::getFrameBuffer() const
{
	return mFrameBuffer;
}

unsigned nex::RenderTargetGL::getLayerFromCubeMapSide(CubeMapSide side)
{
	return (unsigned)side;
}

bool nex::RenderTargetGL::isComplete() const
{
	bool result;
	GLCall(result = glCheckNamedFramebufferStatus(mFrameBuffer, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	return result;
}

bool nex::RenderTargetGL::isDepthType(RenderAttachment::Type type)
{
	static bool table[] = {
		false,
		true,
		true,
		true
	};

	static const size_t size = (size_t)RenderAttachment::Type::LAST - (size_t)RenderAttachment::Type::FIRST + 1;
	static const size_t tableSize = sizeof(table) / sizeof(bool);
	static_assert(tableSize == size, "RenderAttachment::Type and bool table don't match!");

	return table[(unsigned)type];
}


void nex::RenderTargetGL::setFrameBuffer(GLuint newValue)
{
	mFrameBuffer = newValue;
}

void nex::RenderTargetGL::finalizeColorAttachments() const
{
	enableDrawToColorAttachments(true);
}

std::vector<nex::RenderAttachment>& nex::RenderTargetGL::getColorAttachments()
{
	return mColorAttachments;
}

nex::RenderAttachment& nex::RenderTargetGL::getDepthAttachment()
{
	return mDepthAttachment;
}

void nex::RenderTargetGL::useDepthAttachment(RenderAttachment attachment)
{

	if (!isDepthType(attachment.type))
	{
		throw_with_trace(std::runtime_error("nex::RenderTargetGL::useDepthAttachment(): attachment isn't a depth-stencil component!"));
	}


	mDepthAttachment = std::move(attachment);
	updateDepthAttachment();
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
	depth.side = CubeMapSide::POSITIVE_X;

	bind();
	useDepthAttachment(std::move(depth));

	// TODO validate!
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);

	// A depth map only needs depth (z-value) informations; therefore disable any color buffers
	enableDrawToColorAttachments(false);
	enableReadFromColorAttachments(false);

	assertCompletion();

	unbind();
}