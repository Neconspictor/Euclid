#include <nex/renderer/RenderCommandQueue.hpp>
#include <nex/camera/Camera.hpp>
#include "nex/material/Material.hpp"
#include <algorithm>
#include "nex/Scene.hpp"

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
	std::sort(mDeferredCommands.begin(), mDeferredCommands.end(), defaultCompare);
	std::sort(mShadowCommands.begin(), mShadowCommands.end(), defaultCompare);

	auto compareBind = std::bind(&nex::RenderCommandQueue::transparentCompare, this, std::placeholders::_1, std::placeholders::_2);
	std::sort(mForwardCommands.begin(), mForwardCommands.end(), compareBind);
}


// false if fully outside, true if inside or intersects
bool nex::RenderCommandQueue::boxInFrustum(const nex::Frustum& frustum, const nex::AABB& box)
{
	const byte ALL_CORNERS = 8;
	byte out = 0;

	// check box outside/inside of frustum
	for (int i = 0; i < 6; i++)
	{

		const auto& plane = frustum.planes[i];
		glm::vec4 planeAsVec(plane.normal, plane.signedDistance);

		// check all corners against the plane;
		out = 0;
		out += ((dot(planeAsVec, glm::vec4(box.min.x, box.min.y, box.min.z, 1.0f)) < 0.0) ? 1 : 0);
		out += ((dot(planeAsVec, glm::vec4(box.max.x, box.min.y, box.min.z, 1.0f)) < 0.0) ? 1 : 0);
		out += ((dot(planeAsVec, glm::vec4(box.min.x, box.max.y, box.min.z, 1.0f)) < 0.0) ? 1 : 0);
		out += ((dot(planeAsVec, glm::vec4(box.min.x, box.min.y, box.max.z, 1.0f)) < 0.0) ? 1 : 0);
		out += ((dot(planeAsVec, glm::vec4(box.max.x, box.max.y, box.min.z, 1.0f)) < 0.0) ? 1 : 0);
		out += ((dot(planeAsVec, glm::vec4(box.max.x, box.min.y, box.max.z, 1.0f)) < 0.0) ? 1 : 0);
		out += ((dot(planeAsVec, glm::vec4(box.min.x, box.max.y, box.max.z, 1.0f)) < 0.0) ? 1 : 0);
		out += ((dot(planeAsVec, glm::vec4(box.max.x, box.max.y, box.max.z, 1.0f)) < 0.0) ? 1 : 0);
		if (out == ALL_CORNERS) return false;
	}

	// check frustum outside/inside box
	out = 0; for (const auto& corner : frustum.corners) out += ((corner.x > box.max.x) ? 1 : 0); if (out == ALL_CORNERS) return false;
	out = 0; for (const auto& corner : frustum.corners) out += ((corner.x < box.min.x) ? 1 : 0); if (out == ALL_CORNERS) return false;
	out = 0; for (const auto& corner : frustum.corners) out += ((corner.y > box.max.y) ? 1 : 0); if (out == ALL_CORNERS) return false;
	out = 0; for (const auto& corner : frustum.corners) out += ((corner.y < box.min.y) ? 1 : 0); if (out == ALL_CORNERS) return false;
	out = 0; for (const auto& corner : frustum.corners) out += ((corner.z > box.max.z) ? 1 : 0); if (out == ALL_CORNERS) return false;
	out = 0; for (const auto& corner : frustum.corners) out += ((corner.z < box.min.z) ? 1 : 0); if (out == ALL_CORNERS) return false;

	return true;
}
 
bool nex::RenderCommandQueue::isOutsideFrustum(const RenderCommand& command) const
{
	if (mCamera == nullptr) return false;
	return !boxInFrustum(mCamera->getFrustum(), command.boundingBox);
}

bool nex::RenderCommandQueue::defaultCompare(const RenderCommand& a, const RenderCommand& b)
{
	auto* aTechnique = a.material->getTechnique();
	auto* bTechnique = b.material->getTechnique();

	// At first we sort by techniques 
	// We assume that there exists only one instance of each technique. Thus we compare raw pointers.
	if (aTechnique != bTechnique) return aTechnique < bTechnique;

	// now we sort by mesh 
	// Again we assume that meshes can be distinguished by pointers.
	return a.mesh < b.mesh;
}

bool nex::RenderCommandQueue::transparentCompare(const RenderCommand& a, const RenderCommand& b)
{
	// we want to render objects nearer to the camera at first. 
	// We calculate the middle point of the world space AABB from each mesh as a rough approximation for getting the distance.
	// This approach will fail for some special cases, but should be good enough for most scenes.

	auto aMiddle = a.boundingBox.min + (a.boundingBox.max - a.boundingBox.min) * 0.5f;
	auto bMiddle = b.boundingBox.min + (b.boundingBox.max - b.boundingBox.min) * 0.5f;

	auto cameraPosition = mCamera->getPosition();

	auto aDist = length(aMiddle - cameraPosition);
	auto bDist = length(bMiddle - cameraPosition);

	return aDist < bDist;
}