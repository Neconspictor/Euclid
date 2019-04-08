#include <nex/opengl/texture/RenderTargetGL.hpp>
#include <cassert>
#include <nex/util/ExceptionHandling.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <nex/RenderBackend.hpp>
#include <nex/opengl/texture/TextureGL.hpp>
#include <nex/opengl/opengl.hpp>
#include "nex/texture/Attachment.hpp"
#include <nex/texture/Texture.hpp>

using namespace std;
using namespace glm;


GLuint nex::translate(RenderAttachmentType type, unsigned attachIndex)
{
	static AttachmentTypeGL table [] = {
		AttachmentTypeGL::ATTACHMENT_COLOR,
		AttachmentTypeGL::ATTACHMENT_DEPTH,
		AttachmentTypeGL::ATTACHMENT_STENCIL,
		AttachmentTypeGL::ATTACHMNET_DEPTH_STENCIL
	};

	static const size_t size = (size_t)RenderAttachmentType::LAST - (size_t)RenderAttachmentType::FIRST + 1;
	static const size_t tableSize = sizeof(table) / sizeof(AttachmentTypeGL);
	static_assert(tableSize == size, "RenderAttachment and RenderAttachmentTypeGL don't match!");

	GLuint resultBase = (GLuint)table[(unsigned)type];

	if (resultBase == (GLuint)AttachmentTypeGL::ATTACHMENT_COLOR)
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
	Impl(), data(data), mWidth(width), mHeight(height)
{	
	bind();

	RenderAttachment color;
	color.target = TextureTarget::CUBE_MAP;
	color.type = RenderAttachmentType::COLOR;
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
	bind();

	const unsigned index = 0;

	auto& attachment = mColorAttachments[index];
	attachment.mipmapLevel = mipLevel;
	attachment.side = side;
	updateColorAttachment(index);
	
	//TODO
	GLCall(glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}


void nex::CubeRenderTargetGL::resizeForMipMap(unsigned int mipMapLevel) {

	if (!data.generateMipMaps) {
		throw_with_trace(runtime_error("CubeRenderTargetGL::resizeForMipMap(unsigned int): No mip levels generated for this cube render target!"));
	}

	bind();

	unsigned int mipWidth = (unsigned int)(mWidth * std::pow(0.5, mipMapLevel));
	unsigned int mipHeight = (unsigned int)(mHeight * std::pow(0.5, mipMapLevel));

	if (mDepthAttachment->texture)
	{
		auto* impl = mDepthAttachment->texture->getImpl();
		auto renderBuffer = dynamic_cast<RenderBufferGL*>(impl);
		auto texture2D = dynamic_cast<Texture2DGL*>(impl);

		if (renderBuffer)
		{
			renderBuffer->resize(mipWidth, mipHeight);
		} else if (texture2D)
		{
			texture2D->resize(mipWidth, mipHeight);
		}

		updateDepthAttachment();
	}

	//TODO validate!
	//GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, texture));
	//unbind();
}

nex::RenderAttachmentType nex::RenderAttachment::translate(InternFormat format)
{
	static nex::RenderAttachmentType const table[]
	{
		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,

		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,

		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,

		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,

		RenderAttachmentType::COLOR,
		RenderAttachmentType::COLOR,

		RenderAttachmentType::DEPTH_STENCIL,
		RenderAttachmentType::DEPTH_STENCIL,

		RenderAttachmentType::DEPTH,
		RenderAttachmentType::DEPTH,
		RenderAttachmentType::DEPTH,
		RenderAttachmentType::DEPTH,

		RenderAttachmentType::STENCIL
	};

	static const unsigned size = (unsigned)InternFormat::LAST - (unsigned)InternFormat::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: InternFormat and nex::RenderAttachment::Type table don't match!");

	return table[(unsigned)format];
}

nex::RenderTarget::RenderTarget(std::unique_ptr<Impl> impl) : mImpl(std::move(impl))
{
}

nex::RenderTarget::RenderTarget() : mImpl(make_unique<Impl>())
{
}

nex::RenderTarget::~RenderTarget() = default;

void nex::RenderTarget::addColorAttachment(RenderAttachment attachment)
{
	mImpl->addColorAttachment(std::move(attachment));
}

void nex::RenderTarget::assertCompletion() const
{
	mImpl->assertCompletion();
}

void nex::RenderTarget::bind()
{
	mImpl->bind();
}

void nex::RenderTarget::clear(int components) const
{
	int renderComponentsComponentsGL = RenderTarget2DGL::getRenderComponents(components);
	GLCall(glClear(renderComponentsComponentsGL));
}

void nex::RenderTarget::enableDrawToColorAttachments(bool enable) const
{
	mImpl->enableDrawToColorAttachments(enable);
}

void nex::RenderTarget::enableReadFromColorAttachments(bool enable) const
{
	mImpl->enableReadFromColorAttachments(enable);
}

void nex::RenderTarget::finalizeAttachments() const
{
	mImpl->finalizeColorAttachments();
}

std::vector<nex::RenderAttachment>& nex::RenderTarget::getColorAttachments()
{
	return mImpl->getColorAttachments();
}

nex::Texture* nex::RenderTarget::getColorAttachmentTexture(std::size_t attachmentIndex)
{
	return getColorAttachments()[attachmentIndex].texture.get();
}

nex::RenderAttachment* nex::RenderTarget::getDepthAttachment()
{
	return &mImpl->getDepthAttachment();
}

nex::RenderTarget::Impl* nex::RenderTarget::getImpl() const
{
	return mImpl.get();
}

bool nex::RenderTarget::isComplete() const
{
	return mImpl->isComplete();
}

void nex::RenderTarget::setImpl(std::unique_ptr<Impl> impl)
{
	mImpl = std::move(impl);
}

void nex::RenderTarget::unbind() const
{
	mImpl->unbind();
}

void nex::RenderTarget::updateColorAttachment(unsigned index) const
{
	mImpl->updateColorAttachment(index);
}

void nex::RenderTarget::updateDepthAttachment() const
{
	mImpl->updateDepthAttachment();
}

void nex::RenderTarget::useDepthAttachment(RenderAttachment attachment) const
{
	mImpl->useDepthAttachment(std::move(attachment));
}

nex::RenderTarget::Impl::Impl() :
	mDepthAttachment(make_unique<RenderAttachment>()),
	mFrameBuffer(GL_FALSE)
{
	GLCall(glGenFramebuffers(1, &mFrameBuffer));
}


nex::RenderTarget::Impl::Impl(GLuint frameBuffer) :
	mFrameBuffer(frameBuffer)
{
}

nex::RenderTarget::Impl::~Impl()
{
	if (mFrameBuffer != GL_FALSE)
	{
		GLCall(glDeleteFramebuffers(1, &mFrameBuffer));
	}

	mFrameBuffer = GL_FALSE;
}

void nex::RenderTarget::Impl::addColorAttachment(RenderAttachment attachment)
{
	if (attachment.type != RenderAttachmentType::COLOR)
	{
		//throw_with_trace(std::runtime_error("nex::RenderTargetGL::addColorAttachment(): attachment isn't a color component!"));
	}

	mColorAttachments.emplace_back(std::move(attachment));
	updateColorAttachment(static_cast<unsigned>(mColorAttachments.size() - 1));
}

void nex::RenderTarget::Impl::assertCompletion() const
{
	if (!isComplete())
	{
		throw_with_trace(std::runtime_error("RenderTargetGL::assertCompletion(): RenderTargetGL not complete!"));
	}
}

void nex::RenderTarget::Impl::bind() const
{
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer));
}

void nex::RenderTarget::Impl::enableDrawToColorAttachments(bool enable) const
{
	if (enable)
	{
		const auto colorAttachments = calcColorAttachments();
		GLCall(glNamedFramebufferDrawBuffers(mFrameBuffer, static_cast<unsigned>(colorAttachments.size()), colorAttachments.data()));
	}
	else
	{
		GLCall(glNamedFramebufferDrawBuffer(mFrameBuffer, GL_NONE));
	}
}

void nex::RenderTarget::Impl::enableReadFromColorAttachments(bool enable) const
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

void nex::RenderTarget::Impl::unbind() const
{
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void nex::RenderTarget::Impl::updateColorAttachment(unsigned index) const
{
	const auto& attachment = mColorAttachments[index];
	updateAttachment(attachment);
}

void nex::RenderTarget::Impl::updateDepthAttachment() const
{
	updateAttachment(*mDepthAttachment);
}

const nex::RenderAttachment* nex::RenderTarget::Impl::getByIndex(const unsigned colorAttachIndex) const
{
	for (auto& attachment : mColorAttachments)
	{
		if (attachment.colorAttachIndex == colorAttachIndex)
			return &attachment;
	}

	return nullptr;
}

bool nex::RenderTarget::Impl::isArrayTarget(GLenum target)
{
	return target == GL_TEXTURE_1D_ARRAY ||
		target == GL_TEXTURE_2D_ARRAY;
}

void nex::RenderTarget::Impl::updateAttachment(const RenderAttachment& attachment) const
{
	if (attachment.type == RenderAttachmentType::COLOR && (attachment.texture != nullptr))
	{
		// check that there is no other color attachment with the same color attach index
		const auto* firstFound = getByIndex(attachment.colorAttachIndex);

		if (firstFound != &attachment)
		{
			throw_with_trace(std::runtime_error("nex::RenderTargetGL::updateAttachment(): colorAttachIndex is already used by another attachment!"));
		}
	}

	GLuint textureID = 0; // zero for detaching any currently bound texture
	GLenum textureTarget = GL_FALSE;

	if (attachment.texture != nullptr)
	{
		auto* gl = attachment.texture->getImpl();
		textureID = *gl->getTexture();
		textureTarget = (GLenum)gl->getTarget();
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
	else if (isArrayTarget(textureTarget) || textureTarget == GL_TEXTURE_3D)
	{
		//NOTE: OpenGL 4.5 or DSA extension is needed for textures not being array, cube or 3d
		GLCall(glNamedFramebufferTextureLayer(mFrameBuffer, attachmentType, textureID, attachment.mipmapLevel, layer));
		//GLCall(glNamedFramebufferTexture(mFrameBuffer, attachmentType, textureID, attachment.mipmapLevel));
		//glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, GL_TEXTURE_2D, textureID, 0);
	} else if (textureTarget == GL_TEXTURE_CUBE_MAP)
	{

		auto glSide  = GL_TEXTURE_CUBE_MAP_POSITIVE_X + getLayerFromCubeMapSide(attachment.side);
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, glSide, textureID, attachment.mipmapLevel));
	}
	
	else
	{
		GLCall(glNamedFramebufferTexture(mFrameBuffer, attachmentType, textureID, attachment.mipmapLevel));
	}
}

std::vector<GLenum> nex::RenderTarget::Impl::calcColorAttachments() const
{
	std::vector<GLenum> result;
	for (const auto& attachment : mColorAttachments)
	{
		if (attachment.type == RenderAttachmentType::COLOR)
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
	Impl(),
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
Impl(frameBuffer),
mHeight(height),
mWidth(width)
{
}


nex::RenderTarget2D::RenderTarget2D(std::unique_ptr<Impl> impl) : RenderTarget(std::move(impl))
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

nex::Texture2D* nex::RenderTarget2D::getColor0AttachmentTexture()
{
	return static_cast<Texture2D*>(getColorAttachmentTexture(0));
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

GLuint nex::RenderTarget::Impl::getFrameBuffer() const
{
	return mFrameBuffer;
}

unsigned nex::RenderTarget::Impl::getLayerFromCubeMapSide(CubeMapSide side)
{
	return (unsigned)side;
}

bool nex::RenderTarget::Impl::isComplete() const
{
	GLenum result;
	GLCall(result = glCheckNamedFramebufferStatus(mFrameBuffer, GL_FRAMEBUFFER));
	return result == GL_FRAMEBUFFER_COMPLETE;
}

bool nex::RenderTarget::Impl::isDepthType(RenderAttachmentType type)
{
	static bool table[] = {
		false,
		true,
		true,
		true
	};

	static const size_t size = (size_t)RenderAttachmentType::LAST - (size_t)RenderAttachmentType::FIRST + 1;
	static const size_t tableSize = sizeof(table) / sizeof(bool);
	static_assert(tableSize == size, "RenderAttachment::Type and bool table don't match!");

	return table[(unsigned)type];
}


void nex::RenderTarget::Impl::setFrameBuffer(GLuint newValue)
{
	mFrameBuffer = newValue;
}

void nex::RenderTarget::Impl::finalizeColorAttachments() const
{
	enableDrawToColorAttachments(true);
}

std::vector<nex::RenderAttachment>& nex::RenderTarget::Impl::getColorAttachments()
{
	return mColorAttachments;
}

nex::RenderAttachment& nex::RenderTarget::Impl::getDepthAttachment()
{
	return *mDepthAttachment;
}

void nex::RenderTarget::Impl::useDepthAttachment(RenderAttachment attachment)
{

	if (!isDepthType(attachment.type))
	{
		throw_with_trace(std::runtime_error("nex::RenderTargetGL::useDepthAttachment(): attachment isn't a depth-stencil component!"));
	}


	*mDepthAttachment = std::move(attachment);
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
	Impl()
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
	depth.type = RenderAttachmentType::DEPTH;
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