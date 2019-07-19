#include <nex/renderer/RenderCommandQueue.hpp>
#include <nex/camera/Camera.hpp>
#include "nex/material/Material.hpp"
#include <algorithm>
#include "nex/Scene.hpp"
#include <nex/shader/Technique.hpp>
#include <nex/math/Sphere.hpp>

nex::RenderCommandQueue::RenderCommandQueue(Camera* camera) : mCamera(camera)
{
	if (camera) useCameraCulling(mCamera);
	else useSphereCulling(glm::vec3(0.0f), 0.0f);
}

void nex::RenderCommandQueue::clear()
{
	mPbrCommands.clear();
	mForwardCommands.clear();
	mShadowCommands.clear();
	mTechniques.clear();
	mToolCommands.clear();
	mTransparentCommands.clear();
}

nex::RenderCommandQueue::BufferCollection nex::RenderCommandQueue::getCommands(int types)
{
	BufferCollection result;
	if (types & Deferrable) {
		result.push_back(&getDeferrablePbrCommands());
	}

	if (types & Forward) {
		result.push_back(&getForwardCommands());
	}

	if (types & Transparent) {
		result.push_back(&getTransparentCommands());
	}

	if (types & Shadow) {
		result.push_back(&getShadowCommands());
	}

	return result;
}

nex::RenderCommandQueue::Buffer& nex::RenderCommandQueue::getDeferrablePbrCommands()
{
	return mPbrCommands;
}

const nex::RenderCommandQueue::Buffer & nex::RenderCommandQueue::getDeferrablePbrCommands() const
{
	return mPbrCommands;
}

nex::RenderCommandQueue::Buffer& nex::RenderCommandQueue::getForwardCommands() 
{
	return mForwardCommands;
}

const nex::RenderCommandQueue::Buffer & nex::RenderCommandQueue::getForwardCommands() const
{
	return mForwardCommands;
}

std::multimap<unsigned, nex::RenderCommand>& nex::RenderCommandQueue::getToolCommands() 
{
	return mToolCommands;
}

const std::multimap<unsigned, nex::RenderCommand>& nex::RenderCommandQueue::getToolCommands() const
{
	return mToolCommands;
}

nex::RenderCommandQueue::Buffer& nex::RenderCommandQueue::getTransparentCommands()
{
	return mTransparentCommands;
}

const nex::RenderCommandQueue::Buffer & nex::RenderCommandQueue::getTransparentCommands() const
{
	return mTransparentCommands;
}

nex::RenderCommandQueue::Buffer& nex::RenderCommandQueue::getShadowCommands()
{
	return mShadowCommands;
}

const nex::RenderCommandQueue::Buffer & nex::RenderCommandQueue::getShadowCommands() const
{
	return mShadowCommands;
}

std::unordered_set<nex::Technique*>& nex::RenderCommandQueue::getTechniques()
{
	return mTechniques;
}

const std::unordered_set<nex::Technique*>& nex::RenderCommandQueue::getTechniques() const
{
	return mTechniques;
}

void nex::RenderCommandQueue::push(const RenderCommand& command, bool doCulling)
{
	if (!isInRange(doCulling, command)) return;

	bool isPbr = typeid(*command.material).hash_code() == typeid(PbrMaterial).hash_code();

	const auto& state = command.material->getRenderState();

	if (isPbr && !state.doBlend)
	{
		mPbrCommands.emplace_back(command);
	} else if (state.doBlend)
	{
		mTransparentCommands.emplace_back(command);
	} else if (state.isTool)
	{
		mToolCommands.insert(std::pair<unsigned, RenderCommand>(state.toolDrawIndex, std::move(command)));
	} else
	{
		mForwardCommands.emplace_back(command);
	}

	if (state.doShadowCast)
	{
		mShadowCommands.emplace_back(command);
	}

	mTechniques.insert(command.material->getTechnique());
}

void nex::RenderCommandQueue::useCameraCulling(Camera* camera)
{
	mCullingMethod = CullingMethod::FRUSTUM;
	mCamera = camera;
}

void nex::RenderCommandQueue::useSphereCulling(const glm::vec3 & position, float radius)
{
	mCamera = nullptr;
	mCullingMethod = CullingMethod::FRUSTUM_SPHERE;
	mSphereCuller = Sphere(position, radius);
}

void nex::RenderCommandQueue::sort()
{
	std::sort(mPbrCommands.begin(), mPbrCommands.end(), defaultCompare);
	std::sort(mShadowCommands.begin(), mShadowCommands.end(), defaultCompare);

	auto compareBind = std::bind(&nex::RenderCommandQueue::transparentCompare, this, std::placeholders::_1, std::placeholders::_2);
	std::sort(mTransparentCommands.begin(), mTransparentCommands.end(), compareBind);
}


// false if fully outside, true if inside or intersects
bool nex::RenderCommandQueue::boxInFrustum(const nex::Frustum& frustum, const nex::AABB& boxOriginal) const
{
	auto box = boxOriginal;//mCamera->getView() * boxOriginal;

	const byte ALL_CORNERS = 8;
	byte out = 0;

	// check box outside/inside of frustum
	for (int i = 0; i < 6; i++)
	{

		const auto& plane = frustum.planes[i];
		glm::vec4 planeAsVec(plane.mNormal, plane.mSignedDistance);

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
 
bool nex::RenderCommandQueue::isInRange(bool doCulling, const RenderCommand& command) const
{
	if (!doCulling) return true;
	if (mCullingMethod == CullingMethod::FRUSTUM) {
		if (!mCamera) return false;
		// TODO use world space frustum!
		return boxInFrustum(mCamera->getFrustumWorld(), command.boundingBox);
	}
	else {
		return mSphereCuller.intersects(command.boundingBox);
	}
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
	// we want to render objects further to the camera at first. 

	const glm::vec3& positionA = a.worldTrafo[3];
	const glm::vec3& positionB = b.worldTrafo[3];


	const auto& cullPosition = getCullPosition();

	auto aDist = length(positionA - cullPosition);
	auto bDist = length(positionB - cullPosition);

	return aDist > bDist;
}

const glm::vec3 & nex::RenderCommandQueue::getCullPosition() const
{
	if (mCamera) return mCamera->getPosition();
	else return mSphereCuller.origin;
}