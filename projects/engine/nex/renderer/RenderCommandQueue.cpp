#include <nex/renderer/RenderCommandQueue.hpp>
#include <nex/camera/Camera.hpp>
#include "nex/material/Material.hpp"
#include <algorithm>
#include "nex/scene/Scene.hpp"
#include <nex/math/Sphere.hpp>
#include <nex/pbr/PbrProbe.hpp>
#include <nex/mesh/MeshGroup.hpp>

nex::RenderCommandQueue::RenderCommandQueue(Camera* camera) : mCamera(camera)
{
	if (camera) useCameraCulling(mCamera);
	else useSphereCulling(glm::vec3(0.0f), 0.0f);
}

void nex::RenderCommandQueue::clear()
{
	mDeferredPbrCommands.clear();
	mForwardCommands.clear();
	mProbeCommands.clear();
	mShadowCommands.clear();
	mShaders.clear();
	mToolCommands.clear();
	mTransparentCommands.clear();
}

nex::RenderCommandQueue::ConstBufferCollection nex::RenderCommandQueue::getCommands(int types) const
{
	ConstBufferCollection result;
	if (types & Deferrable) {
		result.push_back(&getDeferrablePbrCommands());
	}

	if (types & Forward) {
		result.push_back(&getForwardCommands());
	}

	if (types & Probe) {
		result.push_back(&getProbeCommands());
	}

	if (types & Transparent) {
		result.push_back(&getTransparentCommands());
	}

	if (types & Shadow) {
		result.push_back(&getShadowCommands());
	}

	return result;
}

nex::RenderCommandQueue::Buffer& nex::RenderCommandQueue::getBeforeTransparentCommands()
{
	return mBeforeTransparentCommands;
}

const nex::RenderCommandQueue::Buffer& nex::RenderCommandQueue::getBeforeTransparentCommands() const
{
	return mBeforeTransparentCommands;
}

nex::RenderCommandQueue::Buffer& nex::RenderCommandQueue::getDeferrablePbrCommands()
{
	return mDeferredPbrCommands;
}

const nex::RenderCommandQueue::Buffer & nex::RenderCommandQueue::getDeferrablePbrCommands() const
{
	return mDeferredPbrCommands;
}

nex::RenderCommandQueue::Buffer& nex::RenderCommandQueue::getForwardCommands() 
{
	return mForwardCommands;
}

const nex::RenderCommandQueue::Buffer & nex::RenderCommandQueue::getForwardCommands() const
{
	return mForwardCommands;
}

nex::RenderCommandQueue::Buffer & nex::RenderCommandQueue::getProbeCommands()
{
	return mProbeCommands;
}

const nex::RenderCommandQueue::Buffer & nex::RenderCommandQueue::getProbeCommands() const
{
	return mProbeCommands;
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

std::unordered_set<nex::Shader*>& nex::RenderCommandQueue::getShaders()
{
	return mShaders;
}

const std::unordered_set<nex::Shader*>& nex::RenderCommandQueue::getShaders() const
{
	return mShaders;
}

void nex::RenderCommandQueue::push(const RenderCommand& command, bool doCulling)
{
	auto& pairs = command.batch->getEntries();
	if (pairs.size() == 0) return;
	if (!isInRange(doCulling, command)) return;

	// Note: All meshes of the batch have the same material type!
	auto* material = pairs[0].second;

	const auto& materialTypeID = typeid(*material);

	static auto& pbrMaterialHash = typeid(PbrMaterial);
	static auto& pbrProbeMaterialHash = typeid(PbrProbe::ProbeMaterial);

	bool isPbr = materialTypeID == pbrMaterialHash;
	bool isProbe = materialTypeID == pbrProbeMaterialHash;

	const auto& state = command.batch->getState();

	if (isPbr && !state.doBlend)
	{
		mDeferredPbrCommands.emplace_back(command);
	}

	else if (command.renderBeforeTransparent) 
	{
		mBeforeTransparentCommands.emplace_back(command);
	}
	else if (state.doBlend)
	{
		//command.material->getRenderState().doDepthWrite = false;
		mTransparentCommands.emplace_back(command);
	} else if (isProbe)
	{
		mProbeCommands.emplace_back(command);

	}  else if (state.isTool)
	{
		mToolCommands.insert(std::pair<unsigned, RenderCommand>(state.toolDrawIndex, command));
	} else
	{
		mForwardCommands.emplace_back(command);
	}

	if (state.doShadowCast)
	{
		mShadowCommands.emplace_back(command);
	}

	mShaders.insert(command.batch->getShader());
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
	std::sort(mDeferredPbrCommands.begin(), mDeferredPbrCommands.end(), defaultCompare);
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
		return boxInFrustum(mCamera->getFrustumWorld(), *command.boundingBox);
	}
	else {
		return mSphereCuller.intersects(*command.boundingBox);
	}
}

bool nex::RenderCommandQueue::defaultCompare(const RenderCommand& a, const RenderCommand& b)
{
	auto* aShader = a.batch->getShader();
	auto* bShader = b.batch->getShader();

	// At first we sort by shader 
	// We assume that there exists only one instance of each technique. Thus we compare raw pointers.
	if (aShader != bShader) return aShader < bShader;

	// now we sort by mesh batch
	return a.batch < b.batch;
}


glm::vec3 getTransparentComparePosition(const nex::RenderCommand& c) {
	if (c.boundingBox) return (c.boundingBox->max + c.boundingBox->min) * 0.5f;
	return (*c.worldTrafo)[3];
}

bool nex::RenderCommandQueue::transparentCompare(const RenderCommand& a, const RenderCommand& b)
{
	// we want to render objects further to the camera at first. 

	const glm::vec3& positionA = getTransparentComparePosition(a);
	const glm::vec3& positionB = getTransparentComparePosition(b);


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