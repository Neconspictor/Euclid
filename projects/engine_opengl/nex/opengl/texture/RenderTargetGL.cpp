#include <nex/opengl/texture/RenderTargetGL.hpp>
#include <cassert>
#include <nex/util/ExceptionHandling.hpp>
#include <glm/gtc/matrix_transform.inl>
#include "nex/renderer/RenderBackend.hpp"
#include <nex/opengl/texture/TextureGL.hpp>
#include <nex/opengl/opengl.hpp>
#include "nex/texture/Attachment.hpp"
#include <nex/texture/Texture.hpp>
#include <nex/opengl/texture/TextureGL.hpp>
#include <glm/gtc/type_ptr.inl>
#include "nex/opengl/CacheGL.hpp"
#include <nex/exception/OpenglException.hpp>

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


nex::CubeRenderTarget::CubeRenderTarget(int width, int height, TextureDesc data) : 
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


nex::CubeRenderTargetGL::CubeRenderTargetGL(unsigned width, unsigned height, TextureDesc data, InternalFormat depthFormat) :
	Impl(width, height)
{	
	RenderAttachment color;
	color.target = TextureTarget::CUBE_MAP;
	color.type = RenderAttachmentType::COLOR;
	color.texture = make_unique<CubeMap>(width, height, data);
	addColorAttachment(std::move(color));

	finalizeAttachments();
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
			texture2D->resize(mipWidth, mipHeight,1, false);
		}

		updateDepthAttachment();
	}
}

nex::RenderAttachmentType nex::RenderAttachment::translate(InternalFormat format)
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

	static const unsigned size = (unsigned)InternalFormat::LAST - (unsigned)InternalFormat::FIRST + 1;
	static_assert(sizeof(table) / sizeof(table[0]) == size, "GL error: InternFormat and nex::RenderAttachment::Type table don't match!");

	return table[(unsigned)format];
}

nex::RenderTarget::RenderTarget(std::unique_ptr<Impl> impl) : mImpl(std::move(impl))
{
}

nex::RenderTarget::RenderTarget(unsigned width, unsigned height) : mImpl(make_unique<Impl>(width, height))
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

void nex::RenderTarget::clear(int components, glm::vec4 color, float depth, int stencil) const
{

	// default framebuffer?
	if (mImpl->getFrameBuffer() == GL_FALSE) {
		int renderComponentsComponentsGL = Impl::getRenderComponents(components);
		GLCall(glClear(renderComponentsComponentsGL));
		return;
	}

	if (components & Depth || components & Stencil) {
		GLboolean enabled;
		GLCall(glGetBooleanv(GL_DEPTH_TEST, &enabled));
		//if (!enabled) nex::throw_with_trace(nex::OpenglException("Depth test has to be enabled for clearing depth/stencil"));
		

		GLCall(glGetBooleanv(GL_DEPTH_WRITEMASK, &enabled));
		if (!enabled) nex::throw_with_trace(nex::OpenglException("Depth writing has to be enabled for clearing depth/stencil"));

	}

	if (components & Color)
	{
		for (auto i = 0; i < mImpl->getColorAttachments().size(); ++i)
		{
			// TODO check for each attachment whether it has a float, signed or unsigned integer format!
			GLCall(glClearNamedFramebufferfv(mImpl->getFrameBuffer(), GL_COLOR, i, glm::value_ptr(color)));
		}
	}

	if (components & Depth && components  & Stencil)
	{
		GLCall(glClearNamedFramebufferfi(mImpl->getFrameBuffer(), GL_DEPTH_STENCIL, 0, depth, stencil));
	}
	else
	{
		if (components & Depth)
		{
			GLCall(glClearNamedFramebufferfv(mImpl->getFrameBuffer(), GL_DEPTH, 0, &depth));
		}

		if (components & Stencil)
		{
			GLCall(glClearNamedFramebufferiv(mImpl->getFrameBuffer(), GL_STENCIL, 0, &stencil));
		}
	}
	
}

void nex::RenderTarget::enableDrawToColorAttachments(bool enable) const
{
	mImpl->enableDrawToColorAttachments(enable);
}

void nex::RenderTarget::enableDrawToColorAttachment(unsigned index, bool enable)
{
	mImpl->enableDrawColorAttachment(index, enable);
}

void nex::RenderTarget::enableReadFromColorAttachments() const
{
	mImpl->updateReadFromColorAttachmentList();
}

void nex::RenderTarget::finalizeAttachments() const
{
	mImpl->finalizeAttachments();
}

std::vector<nex::RenderAttachment>& nex::RenderTarget::getColorAttachments()
{
	return mImpl->getColorAttachments();
}

const std::vector<nex::RenderAttachment>& nex::RenderTarget::getColorAttachments() const
{
	return mImpl->getColorAttachments();
}

nex::Texture* nex::RenderTarget::getColorAttachmentTexture(std::size_t attachmentIndex) const
{
	return getColorAttachments()[attachmentIndex].texture.get();
}

nex::RenderAttachment* nex::RenderTarget::getDepthAttachment() const
{
	return &mImpl->getDepthAttachment();
}

nex::RenderTarget::Impl* nex::RenderTarget::getImpl() const
{
	return mImpl.get();
}

unsigned nex::RenderTarget::getWidth() const {
	return mImpl->getWidth();
}

unsigned nex::RenderTarget::getHeight() const {
	return mImpl->getHeight();
}

bool nex::RenderTarget::isComplete() const
{
	return mImpl->isComplete();
}

void nex::RenderTarget::setImpl(std::unique_ptr<Impl> impl)
{
	mImpl = std::move(impl);
}

void nex::RenderTarget::resetAttachments(const std::vector<RenderAttachment>& attachments) {
	mImpl->resetAttachments(attachments);
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

nex::RenderTarget::Impl::Impl(unsigned width, unsigned height) :
	mDepthAttachment(make_unique<RenderAttachment>()),
	mFrameBuffer(GL_FALSE),
	mWidth(width),
	mHeight(height)
{
	GLCall(glGenFramebuffers(1, &mFrameBuffer));

	// Note: We need this as DSA functions might not work correctly if the framebuffer object isn't created 
	bindOnce(mFrameBuffer);

	init();
}


nex::RenderTarget::Impl::Impl(GLuint frameBuffer, unsigned width, unsigned height) :
	mFrameBuffer(frameBuffer), mWidth(width), mHeight(height)
{
	init();
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
	mColorAttachmentReadStatus.push_back(true);
	mColorAttachmentDrawStatus.push_back(true);
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
	GlobalCacheGL::get()->BindFramebuffer(mFrameBuffer, true);
}

void nex::RenderTarget::blit(RenderTarget* dest, const Dimension& sourceDim, int components)
{
	auto self = (RenderTarget::Impl*)getImpl();
	auto other = (RenderTarget::Impl*)dest->getImpl();
	GLint componentsGL = RenderTarget::Impl::getRenderComponents(components);
	self->blit(other, sourceDim, componentsGL);
}

void nex::RenderTarget::Impl::updateDrawColorAttachmentList() const
{
	const auto colorAttachments = calcEnabledDrawColorAttachments();
	GLCall(glNamedFramebufferDrawBuffers(mFrameBuffer, static_cast<unsigned>(colorAttachments.size()), colorAttachments.data()));
}

void nex::RenderTarget::Impl::enableDrawToColorAttachments(bool enable)
{
	// Needed for default and framebuffers with no attachments!
	if (!enable) {
		glNamedFramebufferDrawBuffer(mFrameBuffer, GL_NONE);
	}
	else {
		glNamedFramebufferDrawBuffer(mFrameBuffer, GL_COLOR_ATTACHMENT0);
	}

	glColorMask(enable, enable, enable, enable);

	for (auto i = 0; i < mColorAttachmentDrawStatus.size(); ++i) {
		enableReadColorAttachment(i, enable);
	}
}

void nex::RenderTarget::Impl::enableReadColorAttachment(unsigned index, bool enable)
{
	if (mColorAttachmentReadStatus.size() <= index) return;
	mColorAttachmentReadStatus[index] = enable;
	updateReadFromColorAttachmentList();
}

void nex::RenderTarget::Impl::enableDrawColorAttachment(unsigned index, bool enable)
{
	if (mColorAttachmentDrawStatus.size() <= index) return;

	mColorAttachmentDrawStatus[index] = enable;
	updateDrawColorAttachmentList();
}

void nex::RenderTarget::Impl::updateReadFromColorAttachmentList() const
{
	const auto colorAttachments = calcEnabledReadColorAttachments();

	GLenum buf = 0;

	for (auto buffer : colorAttachments)
	{
		buf |= buffer;
	}

	GLCall(glNamedFramebufferReadBuffer(mFrameBuffer, buf));
}

void nex::RenderTarget::Impl::updateColorAttachment(unsigned index) const
{
	if (mColorAttachments.size() <= index) return;
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
		textureTarget = (GLenum)gl->getTargetGL();
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
	else if (isArrayTarget(textureTarget) || textureTarget == GL_TEXTURE_3D || textureTarget == GL_TEXTURE_CUBE_MAP)
	{
		//NOTE: OpenGL 4.5 or DSA extension is needed for textures not being array, cube or 3d
		GLCall(glNamedFramebufferTextureLayer(mFrameBuffer, attachmentType, textureID, attachment.mipmapLevel, layer));
	} else {
		GLCall(glNamedFramebufferTexture(mFrameBuffer, attachmentType, textureID, attachment.mipmapLevel));
	}
}

std::vector<GLenum> nex::RenderTarget::Impl::calcEnabledReadColorAttachments() const
{
	std::vector<GLenum> result;
	for (auto i = 0; i < mColorAttachments.size(); ++i)
	{
		const auto& attachment = mColorAttachments[i];

		if (attachment.type == RenderAttachmentType::COLOR && mColorAttachmentReadStatus[i])
		{
			const auto glEnum = translate(attachment.type, attachment.colorAttachIndex);
			result.push_back(glEnum);
		}
	}

	return result;
}

std::vector<GLenum> nex::RenderTarget::Impl::calcEnabledDrawColorAttachments() const
{
	std::vector<GLenum> result;

	for (auto i = 0; i < mColorAttachments.size(); ++i)
	{
		const auto& attachment = mColorAttachments[i];

		if (attachment.type == RenderAttachmentType::COLOR && mColorAttachmentDrawStatus[i])
		{
			const auto glEnum = translate(attachment.type, attachment.colorAttachIndex);
			result.push_back(glEnum);
		}
	}

	return result;
}

GLint nex::RenderTarget::Impl::getRenderComponents(int components)
{
	int componentsGL = 0;
	if (components & RenderComponent::Color) componentsGL |= GL_COLOR_BUFFER_BIT;
	if (components & RenderComponent::Depth) componentsGL |= GL_DEPTH_BUFFER_BIT;
	if (components & RenderComponent::Stencil) componentsGL |= GL_STENCIL_BUFFER_BIT;

	return componentsGL;
}

void nex::RenderTarget::Impl::init()
{
	if (mFrameBuffer == GL_FALSE) {
		return;
	}
	GLCall(glNamedFramebufferParameteri(mFrameBuffer, GL_FRAMEBUFFER_DEFAULT_WIDTH, mWidth));
	GLCall(glNamedFramebufferParameteri(mFrameBuffer, GL_FRAMEBUFFER_DEFAULT_HEIGHT, mHeight));
	GLCall(glNamedFramebufferParameteri(mFrameBuffer, GL_FRAMEBUFFER_DEFAULT_LAYERS, 0));
	GLCall(glNamedFramebufferParameteri(mFrameBuffer, GL_FRAMEBUFFER_DEFAULT_SAMPLES, 0));
	GLCall(glNamedFramebufferParameteri(mFrameBuffer, GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS, GL_FALSE));
}

void nex::RenderTarget::Impl::bindOnce(GLuint frameBufferID)
{
	auto* cache = GlobalCacheGL::get();
	auto readBuffer = cache->getActiveReadFrameBuffer();
	auto drawBuffer = cache->getActiveDrawFrameBuffer();

	cache->BindFramebuffer(frameBufferID, true);
	
	cache->BindDrawFramebuffer(drawBuffer, true);
	cache->BindReadFramebuffer(readBuffer, true);
}


nex::RenderTarget2D::RenderTarget2D(std::unique_ptr<Impl> impl) : RenderTarget(std::move(impl))
{
}

nex::RenderTarget2D::RenderTarget2D(int width, int height, const TextureDesc& data, unsigned samples): 
	RenderTarget(make_unique<Impl>(width, height))
{
	const bool isMultiSample = samples > 1;

	RenderAttachment color;

	if (isMultiSample)
	{
		color.target = TextureTarget::TEXTURE2D_MULTISAMPLE;
		color.texture = make_unique<Texture2DMultisample>(width, height, data, samples);
	}
	else
	{
		color.target = TextureTarget::TEXTURE2D;
		color.texture = make_unique<Texture2D>(width, height, data, nullptr);
	}

	addColorAttachment(std::move(color));


	finalizeAttachments();
	assertCompletion();

}

unsigned nex::RenderTarget2D::getWidth() const
{
	return getImpl()->getWidth();
}

unsigned nex::RenderTarget2D::getHeight() const
{
	return getImpl()->getHeight();
}

nex::Texture2D* nex::RenderTarget2D::getColor0AttachmentTexture()
{
	return static_cast<Texture2D*>(getColorAttachmentTexture(0));
}


void nex::RenderTarget::Impl::blit(RenderTarget::Impl* dest, const Dimension& sourceDim, GLuint components) const
{
	GLCall(glBlitNamedFramebuffer(getFrameBuffer(), dest->getFrameBuffer(), 
		sourceDim.xPos, sourceDim.yPos, sourceDim.width, sourceDim.height,
		sourceDim.xPos, sourceDim.yPos, sourceDim.width, sourceDim.height,
		components,
		GL_NEAREST));
}

GLuint nex::RenderTarget::Impl::getFrameBuffer() const
{
	return mFrameBuffer;
}

unsigned nex::RenderTarget::Impl::getLayerFromCubeMapSide(CubeMapSide side)
{
	return (unsigned)side;
}

unsigned nex::RenderTarget::Impl::getWidth() const
{
	return mWidth;
}

unsigned nex::RenderTarget::Impl::getHeight() const
{
	return mHeight;
}

bool nex::RenderTarget::Impl::isComplete() const
{
	GLenum result;
	GLCall(result = glCheckNamedFramebufferStatus(mFrameBuffer, GL_DRAW_FRAMEBUFFER));

	if (result == GL_FRAMEBUFFER_COMPLETE) {
		GLCall(result = glCheckNamedFramebufferStatus(mFrameBuffer, GL_READ_FRAMEBUFFER));
	}

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

void nex::RenderTarget::Impl::resetAttachments(const std::vector<RenderAttachment>& attachments)
{
	mColorAttachments.clear();
	mColorAttachmentReadStatus.clear();
	mColorAttachmentDrawStatus.clear();

	for (const auto& attachment : attachments)
		addColorAttachment(attachment);

	finalizeAttachments();
}

void nex::RenderTarget::Impl::finalizeAttachments() const
{
	updateDrawColorAttachmentList();
	updateReadFromColorAttachmentList();
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
	Impl(width, height)
{

	TextureDesc desc;
	desc.minFilter = TexFilter::Nearest;
	desc.magFilter = TexFilter::Nearest;
	desc.wrapS = desc.wrapR = desc.wrapT = UVTechnique::ClampToEdge;
	desc.internalFormat = InternalFormat::DEPTH_COMPONENT32F;

	RenderAttachment depth;

	depth.texture = make_unique<CubeMap>(width, height, desc);
	depth.type = RenderAttachmentType::DEPTH;
	depth.target = TextureTarget::CUBE_MAP;
	depth.side = CubeMapSide::POSITIVE_X;

	useDepthAttachment(std::move(depth));

	assertCompletion();
}