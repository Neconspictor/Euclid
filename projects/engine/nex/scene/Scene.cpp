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

namespace nex
{
	Scene::Scene() : mHasChanged(false) 
	{
	}

	Scene::~Scene()
	{
	}

	UniqueLock Scene::acquireLock() const {
		return UniqueLock(mMutex);
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
		if (vob->getType() == VobType::Probe)
			mActiveProbeVobs.erase((ProbeVob*)vob);

		auto* updateable = dynamic_cast<FrameUpdateable*>(vob);

		if (updateable) {
			mActiveUpdateables.erase(updateable);
		}
		mHasChanged = true;
	}

	bool Scene::deleteVobUnsafe(Vob* vob)
	{
		auto it = std::remove_if(mVobStore.begin(), mVobStore.end(), [&](auto& v) {
			return v.get() == vob;
			});

		if (it != mVobStore.end()) {
			mVobStore.erase(it);
			mHasChanged = true;
			return true;
		}
			
		return false;
	}

	Vob* Scene::addVobUnsafe(std::unique_ptr<Vob> vob, bool setActive)
	{
		mHasChanged = true;
		mVobStore.emplace_back(std::move(vob));
		auto*  vobPtr = mVobStore.back().get();
		if (setActive)
		{
			addActiveVobUnsafe(vobPtr);
		}
		return vobPtr;
	}

	Vob* Scene::createVobUnsafe(std::list<MeshBatch>* batches, bool setActive)
	{
		mHasChanged = true;
		mVobStore.emplace_back(std::make_unique<Vob>(nullptr, batches));
		auto*  vob = mVobStore.back().get();
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

	const std::vector<std::unique_ptr<Vob>>& Scene::getVobsUnsafe() const
	{
		return mVobStore;
	}

	std::vector<std::unique_ptr<Vob>>& Scene::getVobsUnsafe()
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
		for (const auto& root : getActiveVobsUnsafe())
		{
			mBoundingBox = maxAABB(mBoundingBox, root->getBoundingBox());
		}

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