#include <nex/texture/RenderTarget.hpp>
#include <nex/texture/Attachment.hpp>
#include <nex/texture/Texture.hpp>

nex::RenderTargetSwitcher::RenderTargetSwitcher(RenderTarget* target, unsigned colorAttachIndex,
	std::shared_ptr<Texture> first, std::shared_ptr<Texture> second) :
	mTarget(target),
	mColorAttachIndex(colorAttachIndex),
	mActive(0)
{
	mTextures.resize(2);
	setTextures(std::move(first), std::move(second));
}

void nex::RenderTargetSwitcher::switchTexture(bool update)
{
	mActive = !mActive;

	if (update)
		updateRenderTarget();
}

bool nex::RenderTargetSwitcher::getActive() const
{
	return mActive;
}

nex::Texture* nex::RenderTargetSwitcher::getActiveTexture()
{
	return mTextures[mActive].get();
}

nex::Texture* nex::RenderTargetSwitcher::getNonActiveTexture()
{
	return mTextures[!mActive].get();
}

const std::vector<std::shared_ptr<nex::Texture>>& nex::RenderTargetSwitcher::getTextures() const
{
	return mTextures;
}

void nex::RenderTargetSwitcher::setActive(bool active, bool update)
{
	mActive = active;

	if (update)
		updateRenderTarget();
}

void nex::RenderTargetSwitcher::setTarget(RenderTarget* target, bool update)
{
	mTarget = target;
	if (update)
		updateRenderTarget();
}

void nex::RenderTargetSwitcher::setTextures(std::shared_ptr<Texture> first, std::shared_ptr<Texture> second,
	bool update)
{
	mTextures[0] = first;
	mTextures[1] = second;

	if (update)
		updateRenderTarget();
}

void nex::RenderTargetSwitcher::updateRenderTarget()
{
	if (!mTarget) return;
	mTarget->getColorAttachments()[mColorAttachIndex].texture = mTextures[mActive];
	mTarget->updateColorAttachment(mColorAttachIndex);
}