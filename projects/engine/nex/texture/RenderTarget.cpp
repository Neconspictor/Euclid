#include <nex/texture/RenderTarget.hpp>
#include <nex/texture/Texture.hpp>

nex::Texture* nex::RenderTargetImpl::getTexture()
{
	return mRenderResult.get();
}

void nex::RenderTargetImpl::setTexture(Texture* texture)
{
	mRenderResult.reset();
	mRenderResult = texture;
}

nex::RenderTargetImpl::RenderTargetImpl(unsigned width, unsigned height) : mWidth(width), mHeight(height), mDepthStencilMap(nullptr)
{
	mRenderResult = Texture::create();
}


nex::RenderTarget::~RenderTarget()
{
	delete mImpl;
	mImpl = nullptr;
}

nex::RenderTargetImpl* nex::RenderTarget::getImpl() const
{
	return mImpl;
}

nex::Texture* nex::RenderTarget::getTexture()
{
	return mImpl->getTexture();
}

unsigned nex::RenderTarget::getHeight() const
{
	return mImpl->mHeight;
}

unsigned nex::RenderTarget::getWidth() const
{
	return mImpl->mWidth;
}

nex::Texture* nex::RenderTarget::getDepthStencilMap()
{
	return mImpl->mDepthStencilMap;
}

void nex::RenderTarget::setTexture(Texture* texture)
{
	mImpl->setTexture(texture);
}

void nex::RenderTarget::validateDepthStencilMap(Texture* texture)
{
	const bool isDepthStencil = dynamic_cast<DepthStencilMap*>(texture) != nullptr;
	const bool isRenderBuffer = dynamic_cast<RenderBuffer*>(texture) != nullptr;

	if (!isDepthStencil && !isRenderBuffer)
		throw std::runtime_error("nex::RenderTarget::validateDepthStencilMap failed: Wrong texture input!");
}

void nex::RenderTarget::useDepthStencilMap(Texture* depthStencilMap)
{
	mImpl->useDepthStencilMap(depthStencilMap);
}

int nex::CubeRenderTarget::getHeightMipLevel(unsigned mipMapLevel) const
{
	return (int)(getHeight() * std::pow(0.5, mipMapLevel));
}

int nex::CubeRenderTarget::getWidthMipLevel(unsigned mipMapLevel) const
{
	return (int)(getWidth() * std::pow(0.5, mipMapLevel));
}