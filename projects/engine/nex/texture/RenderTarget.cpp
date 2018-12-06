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

nex::RenderTargetImpl::RenderTargetImpl(unsigned width, unsigned height)
{
	mRenderResult = Texture::create();
	mRenderResult->setWidth(width);
	mRenderResult->setHeight(height);
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

void nex::RenderTarget::setTexture(Texture* texture)
{
	mImpl->setTexture(texture);
}

int nex::CubeRenderTarget::getHeightMipLevel(unsigned mipMapLevel) const
{
	return (int)(getHeight() * std::pow(0.5, mipMapLevel));
}

int nex::CubeRenderTarget::getWidthMipLevel(unsigned mipMapLevel) const
{
	return (int)(getWidth() * std::pow(0.5, mipMapLevel));
}