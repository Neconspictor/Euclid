#include <nex/texture/RenderTarget.hpp>
#include <nex/texture/Attachment.hpp>
#include <nex/texture/Texture.hpp>

nex::RenderTargetSwitcher::RenderTargetSwitcher(RenderTarget* target, unsigned colorAttachIndex,
	std::shared_ptr<Texture> first, std::shared_ptr<Texture> second) :
	mTarget(target),
	mColorAttachIndex(colorAttachIndex)
{
	mTextures.resize(2);
	mTextures[0] = first;
	mTextures[1] = second;
}

void nex::RenderTargetSwitcher::switchTexture()
{
	mActive = !mActive;

	//update render target
	mTarget->getColorAttachments()[mColorAttachIndex].texture = mTextures[mActive];
	mTarget->updateColorAttachment(mColorAttachIndex);
}

bool nex::RenderTargetSwitcher::getActive() const
{
	return mActive;
}

const std::vector<std::shared_ptr<nex::Texture>>& nex::RenderTargetSwitcher::getTextures() const
{
	return mTextures;
}

void nex::RenderTargetSwitcher::setActive(bool active)
{
	mActive = active;
}