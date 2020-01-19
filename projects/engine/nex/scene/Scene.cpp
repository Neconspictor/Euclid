#include <nex/scene/Scene.hpp>
#include <nex/scene/Vob.hpp>
#include <queue>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <nex/math/Math.hpp>
#include <nex/mesh/Mesh.hpp>
#include <nex/mesh/MeshGroup.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <nex/renderer/RenderCommandQueue.hpp>
#include <nex/anim/AnimationManager.hpp>
#include <nex/GI/PbrProbe.hpp>

namespace nex
{
	Scene::Scene() : mHasChanged(false) 
	{
	}

	Scene::~Scene()
	{
	}

	UniqueLock Scene::acquireLock() const {
		return UniqueLock(mMutex); // Copy-elision prevents releasing in this function.
	}

	void Scene::addActiveVobUnsafe(Vob* vob, bool recursive)
	{
		if (vob->isRoot()) {
			mActiveRoots.push_back(vob);
		}
	
		mActiveVobsFlat.push_back(vob);

		if (recursive) {
			for (auto* child : vob->getChildren()) {
				addActiveVobUnsafe(child);
			}
		}

		if (auto* probeVob = dynamic_cast<ProbeVob*>(vob)) {
			mActiveProbeVobs.push_back(probeVob);
		}

		if (auto* updateable = dynamic_cast<FrameUpdateable*>(vob)) {
			mActiveUpdateables.insert(updateable);
		}

		mHasChanged = true;
	}

	void Scene::removeActiveVobUnsafe(Vob* vob, bool recursive)
	{
		mActiveRoots.erase(std::remove(mActiveRoots.begin(), mActiveRoots.end(), vob), mActiveRoots.end());
		
		mActiveProbeVobs.erase(std::remove(mActiveProbeVobs.begin(), mActiveProbeVobs.end(), dynamic_cast<ProbeVob*>(vob)), mActiveProbeVobs.end());
		mActiveUpdateables.erase(dynamic_cast<FrameUpdateable*>(vob));

		mActiveVobsFlat.erase(std::remove(mActiveVobsFlat.begin(), mActiveVobsFlat.end(), vob), mActiveVobsFlat.end());

		if (recursive) {
			for (auto* child : vob->getChildren()) {
				removeActiveVobUnsafe(child);
			}
		}

		mHasChanged = true;
	}

	bool Scene::deleteVobUnsafe(Vob* vob, bool recursive)
	{
		if (recursive) {
			for (auto* child : vob->getChildren())
				deleteVobUnsafe(child);
		}

		//we don't use std::remove_if since it potentially frees memory
		auto it = std::find_if(mVobStore.begin(), mVobStore.end(), [&](auto& v) {
			return v.get() == vob;
		});

		if (auto* parent = vob->getParent()) {

			parent->removeChild(vob);
		}

		removeActiveVobUnsafe(vob);

		if (it != mVobStore.end()) {
			mVobStore.erase(it);
		
		}
			
		mHasChanged = true;
		return true;
	}

	Vob* Scene::addVobUnsafe(std::unique_ptr<Vob> vob, bool setActive)
	{
		mHasChanged = true;
		auto*  vobPtr = vob.get();
		mVobStore.insert(std::move(vob));
		if (setActive)
		{
			addActiveVobUnsafe(vobPtr);
		}
		return vobPtr;
	}

	Vob* Scene::createVobUnsafe(std::vector<MeshBatch>* batches, bool setActive)
	{
		mHasChanged = true;
		auto v = std::make_unique<Vob>(nullptr);
		v->setBatches(batches);
		auto* vob = v.get();
		mVobStore.insert(std::move(v));
		
		if (setActive)
		{
			addActiveVobUnsafe(vob);
		}
		return vob;
	}

	void Scene::frameUpdate(const RenderContext& constants)
	{
		for (auto* updateable : mActiveUpdateables) {
			updateable->frameUpdate(constants);
		}
	}

	const nex::Scene::VobStore& Scene::getVobsUnsafe() const
	{
		return mVobStore;
	}

	nex::Scene::VobStore& Scene::getVobsUnsafe()
	{
		return mVobStore;
	}

	bool Scene::hasChangedUnsafe() const
	{
		return mHasChanged;
	}

	void Scene::clearUnsafe()
	{
		mActiveRoots.clear();
		mActiveVobsFlat.clear();
		mActiveUpdateables.clear();
		mVobStore.clear();
		mHasChanged = true;
	}

	const Scene::VobRange& Scene::getActiveVobsUnsafe() const
	{
		return mActiveVobsFlat;
	}

	nex::Scene::VobRange& Scene::getActiveRootsUnsafe()
	{
		return mActiveRoots;
	}

	const nex::Scene::VobRange& Scene::getActiveRootsUnsafe() const
	{
		return mActiveRoots;
	}

	void Scene::removeActiveRoot(Vob* vob)
	{
		mActiveRoots.erase(std::remove(mActiveRoots.begin(), mActiveRoots.end(), vob), mActiveRoots.end());
	}

	const Scene::FrameUpdateableRange& Scene::getActiveFrameUpdateables() const
	{
		return mActiveUpdateables;
	}

	const Scene::ProbeRange& Scene::getActiveProbeVobsUnsafe() const
	{
		return mActiveProbeVobs;
	}

	void Scene::updateWorldTrafoHierarchyUnsafe(bool resetPrevWorldTrafo)
	{
		for (auto& vob : mActiveRoots)
			vob->updateTrafo(resetPrevWorldTrafo);

		mHasChanged = true;
	}

	void Scene::calcSceneBoundingBoxUnsafe()
	{
		mBoundingBox = AABB();

		for (const auto& root : getActiveVobsUnsafe())
		{
			mBoundingBox = maxAABB(mBoundingBox, root->getBoundingBoxWorld());
		}

		//mBoundingBox.min = glm::vec3(-25.0f);
		//mBoundingBox.max = glm::vec3(25.0f);

		mHasChanged = true;
	}

	const AABB& Scene::getSceneBoundingBox() const
	{
		return mBoundingBox;
	}

	void Scene::collectRenderCommands(RenderCommandQueue& queue, bool doCulling, ShaderStorageBuffer* boneTrafoBuffer)
	{
		auto guard = acquireLock();

		for (auto* vob : getActiveVobsUnsafe())
		{
			vob->collectRenderCommands(queue, doCulling, boneTrafoBuffer);
		}
	}

	void Scene::setHasChangedUnsafe(bool changed)
	{
		mHasChanged = changed;
	}
}