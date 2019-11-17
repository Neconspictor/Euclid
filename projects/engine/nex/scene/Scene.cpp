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
	SceneNode::SceneNode() noexcept : mMesh(nullptr), mMaterial(nullptr),
		mParent(nullptr)
	{
	}

	SceneNode::~SceneNode()
	{
		clear();
	}

	void SceneNode::addChild(SceneNode* node)
	{
		mChildren.push_back(node);
		node->mParent = this;
	}

	void SceneNode::clear()
	{
		for (auto* child : mChildren) {
			delete child;
		}
		mChildren.clear();
	}

	const SceneNode::Children& SceneNode::getChildren() const
	{
		return  mChildren;
	}

	Mesh* SceneNode::getMesh() const
	{
		return mMesh;
	}

	Material* SceneNode::getMaterial() const
	{
		return mMaterial;
	}

	const nex::AABB & SceneNode::getMeshBoundingBoxWorld() const
	{
		return mBoundingBox;
	}

	SceneNode* SceneNode::getParent()
	{
		return mParent;
	}

	const glm::mat4& SceneNode::getWorldTrafo() const
	{
		return mWorldTrafo;
	}

	const glm::mat4& SceneNode::getPrevWorldTrafo() const
	{
		return mPrevWorldTrafo;
	}

	void SceneNode::setMesh(Mesh* mesh)
	{
		mMesh = mesh;
	}

	void SceneNode::setMaterial(Material* material)
	{
		mMaterial = material;
	}

	void SceneNode::setParent(SceneNode* parent)
	{
		mParent = parent;
	}

	void SceneNode::updateChildrenWorldTrafos(bool resetPrevWorldTrafo)
	{
		//assume that the world trafo of the current node is up to date (needs no update from the parent)
		auto* backup = mParent;
		mParent = nullptr;
		updateWorldTrafoHierarchy(resetPrevWorldTrafo);
		mParent = backup;
	}

	void SceneNode::updateWorldTrafoHierarchy(bool resetPrevWorldTrafo)
	{
		std::queue<SceneNode*> queue;
		queue.push(this);

		while (!queue.empty())
		{
			auto* node = queue.front();
			queue.pop();

			node->updateWorldTrafo(resetPrevWorldTrafo);

			if (node->mMesh) {
				node->mBoundingBox = node->mWorldTrafo * node->mMesh->getAABB();
			}

			const auto& children = node->getChildren();

			for (auto& child : children)
				queue.push(child);
		}
	}

	void SceneNode::setLocalTrafo(const glm::mat4& mat)
	{
		mLocalTrafo = mat;
	}

	void SceneNode::updateWorldTrafo(bool resetPrevWorldTrafo)
	{
		if (!resetPrevWorldTrafo)
		{
			mPrevWorldTrafo = mWorldTrafo;
		}

		if (mParent)
		{
			mWorldTrafo = mParent->mWorldTrafo * mLocalTrafo;
		} else
		{
			mWorldTrafo = mLocalTrafo;
		}

		if (resetPrevWorldTrafo)
			mPrevWorldTrafo = mWorldTrafo;
	}

	Scene::Scene() : mHasChanged(false) 
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
		mHasChanged = true;
	}

	void Scene::removeActiveVobUnsafe(Vob* vob)
	{
		mActiveVobs.erase(vob);
		if (vob->getType() == VobType::Probe)
			mActiveProbeVobs.erase((ProbeVob*)vob);
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

	Vob* Scene::createVobUnsafe(SceneNode* meshRootNode, bool setActive)
	{
		mHasChanged = true;
		mVobStore.emplace_back(std::make_unique<Vob>(meshRootNode));
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
		mVobStore.clear();
		mHasChanged = true;
	}

	const Scene::VobRange& Scene::getActiveVobsUnsafe() const
	{
		return mActiveVobs;
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

	void Scene::collectRenderCommands(RenderCommandQueue& commandQueue, bool doCulling) const
	{
		RenderCommand command;
		std::list<SceneNode*> queue;


		acquireLock();
		for (const auto& root : getActiveVobsUnsafe())
		{
			queue.push_back(root->getMeshRootNode());

			while (!queue.empty())
			{
				auto* node = queue.front();
				queue.pop_front();

				auto range = node->getChildren();

				for (auto* node : range)
				{
					queue.push_back(node);
				}

				auto* mesh = node->getMesh();
				if (mesh != nullptr)
				{
					command.mesh = mesh;
					command.material = node->getMaterial();
					command.worldTrafo = &node->getWorldTrafo();
					command.prevWorldTrafo = &node->getPrevWorldTrafo();
					command.boundingBox = &node->getMeshBoundingBoxWorld();
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