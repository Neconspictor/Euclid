#include <nex/renderer/RenderCommandQueue.hpp>
#include <nex/camera/Camera.hpp>
#include "nex/material/Material.hpp"

nex::RenderCommandQueue::RenderCommandQueue(Camera* camera) : mCamera(camera)
{
}

void nex::RenderCommandQueue::clear()
{
	mDeferredCommands.clear();
	mForwardCommands.clear();
	mShadowCommands.clear();
}

const std::vector<nex::RenderCommand>& nex::RenderCommandQueue::getDeferredCommands() const
{
	return mDeferredCommands;
}

const std::vector<nex::RenderCommand>& nex::RenderCommandQueue::getForwardCommands() const
{
	return mForwardCommands;
}

const std::vector<nex::RenderCommand>& nex::RenderCommandQueue::getShadowCommands() const
{
	return mShadowCommands;
}

void nex::RenderCommandQueue::push(const RenderCommand& command, bool cull)
{
	if (cull && isOutsideFrustum(command)) return;

	const auto& state = command.material->getRenderState();

	if (state.doBlend)
	{
		mForwardCommands.emplace_back(command);
	}
	else
	{
		mDeferredCommands.emplace_back(command);
	}

	if (state.doShadowCast)
	{
		mShadowCommands.emplace_back(command);
	}
}

void nex::RenderCommandQueue::setCamera(Camera* camera)
{
	mCamera = camera;
}

void nex::RenderCommandQueue::sort()
{
	//TODO
}
 
bool nex::RenderCommandQueue::isOutsideFrustum(const RenderCommand& command) const
{
	if (mCamera == nullptr) return false;

	//TODO
	return false;
}
