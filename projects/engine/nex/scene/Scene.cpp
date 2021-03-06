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
#include <nex/GI/Probe.hpp>

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
			for (auto& child : vob->getChildren()) {
				addActiveVobUnsafe(child.get());
			}
		}

		if (auto* probeVob = dynamic_cast<ProbeVob*>(vob)) {
			mActiveProbeVobs.push_back(probeVob);
		}

		if (!vob->isStatic()) {
			mActiveUpdateables.insert(vob);
		}

		mHasChanged = true;
	}

	void Scene::removeActiveVobUnsafe(Vob* vob, bool recursive)
	{
		mActiveRoots.erase(std::remove(mActiveRoots.begin(), mActiveRoots.end(), vob), mActiveRoots.end());
		
		mActiveProbeVobs.erase(std::remove(mActiveProbeVobs.begin(), mActiveProbeVobs.end(), dynamic_cast<ProbeVob*>(vob)), mActiveProbeVobs.end());
		mActiveUpdateables.erase(vob);

		mActiveVobsFlat.erase(std::remove(mActiveVobsFlat.begin(), mActiveVobsFlat.end(), vob), mActiveVobsFlat.end());

		if (recursive) {
			for (auto& child : vob->getChildren()) {
				removeActiveVobUnsafe(child.get());
			}
		}

		mHasChanged = true;
	}

	nex::flexible_ptr<Vob> Scene::deleteVobUnsafe(Vob* vob, bool recursive)
	{
		deregister(vob);

		nex::flexible_ptr<Vob> vobManaged;
		// we have to remove it from parent before we delete
		if (auto* parent = vob->getParent()) {

			vobManaged = parent->removeChild(vob);
		}
		
		deleteVobPrivate(vob, recursive);
			
		mHasChanged = true;
		return vobManaged;
	}

	Vob* Scene::addVobUnsafe(VobPtrType vob, bool setActive)
	{
		mHasChanged = true;
		auto*  vobPtr = vob.get();

		std::cout << "Scene::addVobUnsafe: mVobStore.size() before: " << mVobStore.size() << std::endl;
		mVobStore.insert(std::move(vob));
		std::cout << "Scene::addVobUnsafe: mVobStore.size() after: " << mVobStore.size() << std::endl;

		if (auto* resizable = dynamic_cast<Resizable*> (vobPtr)) {
			mResizables.insert(resizable);
		}

		if (setActive)
		{
			addActiveVobUnsafe(vobPtr);
		}

		return vobPtr;
	}

	Vob* Scene::createVobUnsafe(nex::MeshGroup* group, bool setActive)
	{
		mHasChanged = true;
		auto v = std::make_unique<Vob>();
		v->setMeshGroup(nex::make_not_owning(group));
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
		for (auto* vob : mActiveUpdateables) {
			if (vob->isVisible())
				vob->frameUpdate(constants);
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

	void Scene::resize(unsigned width, unsigned height)
	{
		for (auto* resizable : mResizables) {
			resizable->resize(width, height);
		}
	}

	void Scene::clearUnsafe()
	{
		mActiveRoots.clear();
		mActiveVobsFlat.clear();
		mActiveUpdateables.clear();
		mResizables.clear();
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

	void Scene::deregister(Vob* vob)
	{
		try {
			removeActiveVobUnsafe(vob);
			mResizables.erase(dynamic_cast<Resizable*>(vob));
		}
		catch (std::__non_rtti_object & e) {
			throw_with_trace(e);
		}
	}

	void Scene::deleteVobPrivate(Vob* vob, bool recursive)
	{
		if (recursive) {
			for (auto& child : vob->getChildren()) {
				deleteVobPrivate(child.get(), recursive);
			}
		}

		//we don't use std::remove_if since it potentially frees memory
		auto it = std::find_if(mVobStore.begin(), mVobStore.end(), [&](auto& v) {
			return v.get() == vob;
			});

		if (it != mVobStore.end()) {
			mVobStore.erase(it);
		}
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

	void Scene::collectRenderCommands(RenderCommandQueue& queue, bool doCulling, const RenderContext& renderContext) const
	{
		auto guard = acquireLock();

		for (auto* vob : getActiveVobsUnsafe())
		{
			if (!vob->isVisible()) continue;
			vob->collectRenderCommands(queue, doCulling, renderContext);
		}
	}

	void Scene::collectRenderCommands(RenderCommandQueue& queue, bool doCulling, const RenderContext& renderContext, std::function<bool(Vob*)> filter) const
	{
		auto guard = acquireLock();

		for (auto* vob : getActiveVobsUnsafe())
		{
			if (!vob->isVisible()) continue;
			if (filter(vob)) vob->collectRenderCommands(queue, doCulling, renderContext);
		}
	}

	void Scene::setHasChangedUnsafe(bool changed)
	{
		mHasChanged = changed;
	}
}