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
#include <nex/pbr/PbrProbe.hpp>

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

	void Scene::addActiveVobUnsafe(Vob* vob)
	{
		mActiveVobs.insert(vob);
		if (vob->getType() == VobType::Probe)
			mActiveProbeVobs.insert((ProbeVob*)vob);

		auto* updateable = dynamic_cast<FrameUpdateable*>(vob);

		if (updateable) {
			mActiveUpdateables.insert(updateable);
		}

		mHasChanged = true;
	}

	void Scene::removeActiveVobUnsafe(Vob* vob)
	{
		mActiveVobs.erase(vob);
		mActiveProbeVobs.erase(dynamic_cast<ProbeVob*>(vob));
		mActiveUpdateables.erase(dynamic_cast<FrameUpdateable*>(vob));
		mHasChanged = true;
	}

	bool Scene::deleteVobUnsafe(Vob* vob)
	{

		//we don't use std::remove_if since it potentially frees memory
		auto it = std::find_if(mVobStore.begin(), mVobStore.end(), [&](auto& v) {
			return v.get() == vob;
		});

		if (it != mVobStore.end()) {
			removeActiveVobUnsafe(vob);
			mVobStore.erase(it);
			mHasChanged = true;
			return true;
		}
			
		return false;
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

	Vob* Scene::createVobUnsafe(std::list<MeshBatch>* batches, bool setActive)
	{
		mHasChanged = true;
		auto v = std::make_unique<Vob>(nullptr, batches);
		auto* vob = v.get();
		mVobStore.insert(std::move(v));
		
		if (setActive)
		{
			addActiveVobUnsafe(vob);
		}
		return vob;
	}

	void Scene::frameUpdate(const Constants& constants)
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

	bool Scene::isActive(Vob* vob) const
	{
		return mActiveVobs.find(vob) != mActiveVobs.end();
	}

	void Scene::clearUnsafe()
	{
		mActiveVobs.clear();
		mActiveUpdateables.clear();
		mVobStore.clear();
		mHasChanged = true;
	}

	const Scene::VobRange& Scene::getActiveVobsUnsafe() const
	{
		return mActiveVobs;
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
		for (auto& vob : mActiveVobs)
			vob->updateTrafo(resetPrevWorldTrafo);

		mHasChanged = true;
	}

	void Scene::calcSceneBoundingBoxUnsafe()
	{
		mBoundingBox = AABB();

		for (const auto& root : getActiveVobsUnsafe())
		{
			mBoundingBox = maxAABB(mBoundingBox, root->getWorldTrafo() * root->getBoundingBox());
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
		std::list<Vob*> vobQueue;

		auto guard = acquireLock();

		for (auto* vob : mActiveVobs)
		{
			vobQueue.push_back(vob);

			while (!vobQueue.empty())
			{
				auto* vob = vobQueue.front();
				vobQueue.pop_front();

				for (auto* child : vob->getChildren()) {
					vobQueue.push_back(child);
				}

				vob->collectRenderCommands(queue, doCulling, boneTrafoBuffer);
			}
		}
	}

	void Scene::setHasChangedUnsafe(bool changed)
	{
		mHasChanged = changed;
	}
}