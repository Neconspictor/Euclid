#include <nex/texture/RenderTarget.hpp>
#include <nex/texture/Texture.hpp>


nex::Texture* nex::RenderTargetImpl::getTexture()
{
	return mRenderResult;
}

nex::RenderTarget::~RenderTarget()
{
	delete mImpl;
	mImpl = nullptr;
}

int nex::CubeRenderTarget::getHeightMipLevel(unsigned mipMapLevel) const
{
	return (int)(getHeight() * std::pow(0.5, mipMapLevel));
}

int nex::CubeRenderTarget::getWidthMipLevel(unsigned mipMapLevel) const
{
	return (int)(getWidth() * std::pow(0.5, mipMapLevel));
}