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

	void Scene::collectRenderCommands(RenderCommandQueue& commandQueue, bool doCulling, ShaderStorageBuffer* boneTrafoBuffer) const
	{
		RenderCommand command;
		std::list<const Vob*> queue;

		auto guard = acquireLock();

		for (const auto* vob : getActiveVobsUnsafe())
		{
			queue.push_back(vob);


			auto* riggedVob = vob->getType() == VobType::Skinned ? (const RiggedVob*)vob : nullptr;
			bool hasBoneAnimations = riggedVob != nullptr;

			while (!queue.empty())
			{
				auto* vob = queue.front();
				queue.pop_front();

				for (auto* child : vob->getChildren()) {
					queue.push_back(child);
				}

				auto* batches = vob->getBatches();
				if (!batches) continue;

				for (const auto& batch : *batches) {
					command.batch = &batch;
					command.worldTrafo = &vob->getWorldTrafo();
					command.prevWorldTrafo = &vob->getPrevWorldTrafo();
					command.boundingBox = &vob->getBoundingBox();

					if (hasBoneAnimations) {
						command.isBoneAnimated = true;
						command.bones = &riggedVob->getBoneTrafos();
						command.boneBuffer = boneTrafoBuffer;
					}
					else {
						command.isBoneAnimated = false;
						command.bones = nullptr;
						command.boneBuffer = nullptr;
					}

					commandQueue.push(command, doCulling);
				}
			}
		}
	}

	void Scene::setHasChangedUnsafe(bool changed)
	{
		mHasChanged = changed;
	}
}