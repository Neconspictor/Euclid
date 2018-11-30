#include <nex/texture/RenderTarget.hpp>
#include <nex/texture/Texture.hpp>

nex::RenderTarget::~RenderTarget()
{
	delete mImpl;
	mImpl = nullptr;
	delete mRenderResult;
	mRenderResult = nullptr;
}

nex::Texture* nex::RenderTarget::getTexture()
{
	return mRenderResult;
}

int nex::RenderTarget::getHeight() const
{
	return mRenderResult->getHeight();
}

int nex::RenderTarget::getWidth() const
{
	return mRenderResult->getWidth();
}

void nex::RenderTarget::setTexture(Texture* texture)
{
	mRenderResult = texture;
}

int nex::CubeRenderTarget::getHeightMipLevel(unsigned mipMapLevel) const
{
	return (int)(mRenderResult->getHeight() * std::pow(0.5, mipMapLevel));
}

int nex::CubeRenderTarget::getWidthMipLevel(unsigned mipMapLevel) const
{
	return (int)(mRenderResult->getWidth() * std::pow(0.5, mipMapLevel));
}
