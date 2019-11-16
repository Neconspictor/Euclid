#include <nex/Scene.hpp>
#include <queue>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <nex/math/Math.hpp>
#include <nex/mesh/Mesh.hpp>
#include <nex/mesh/MeshContainer.hpp>
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
					command.worldTrafo = node->getWorldTrafo();
					command.prevWorldTrafo = node->getPrevWorldTrafo();
					command.boundingBox = node->getMeshBoundingBoxWorld();
					commandQueue.push(command, doCulling);
				}
			}
		}
	}

	void Scene::setHasChangedUnsafe(bool changed)
	{
		mHasChanged = changed;
	}


	Vob::Vob(SceneNode* meshRootNode) : mMeshRootNode(meshRootNode), mPosition(0.0f), mRotation(glm::quat()), mScale(1.0f), 
		mSelectable(true), mIsDeletable(true),
		mType(VobType::Normal)
	{

	}

	Vob::~Vob()
	{
		if (mMeshRootNode) delete mMeshRootNode;
		mMeshRootNode = nullptr;
	}

	const SceneNode* Vob::getMeshRootNode() const
	{
		return mMeshRootNode;
	}

	SceneNode* Vob::getMeshRootNode()
	{
		return mMeshRootNode;
	}

	const AABB& Vob::getBoundingBox() const
	{
		return mBoundingBox;
	}

	const glm::vec3& Vob::getPosition() const
	{
		return mPosition;
	}

	const glm::quat& Vob::getRotation() const
	{
		return mRotation;
	}

	const glm::vec3& Vob::getScale() const
	{
		return mScale;
	}

	bool Vob::getSelectable() const
	{
		return mSelectable;
	}

	VobType Vob::getType() const
	{
		return mType;
	}

	bool Vob::isDeletable() const
	{
		return mIsDeletable;
	}

	void Vob::rotateGlobal(const glm::vec3& axisWorld, float angle)
	{
		mRotation = glm::normalize(glm::rotate(mRotation, angle, inverse(mRotation) * axisWorld));
	}

	void Vob::rotateGlobal(const glm::vec3& eulerAngles)
	{
		mRotation = glm::normalize(glm::rotate(mRotation, eulerAngles.x, inverse(mRotation) * glm::vec3(1, 0, 0)));
		mRotation = glm::normalize(glm::rotate(mRotation, eulerAngles.y, inverse(mRotation) * glm::vec3(0, 1, 0)));
		mRotation = glm::normalize(glm::rotate(mRotation, eulerAngles.z, inverse(mRotation) * glm::vec3(0, 0, 1.0f)));
	}

	void Vob::rotateLocal(const glm::vec3& eulerAngles)
	{
		mRotation = glm::normalize(glm::rotate(mRotation, eulerAngles.x, glm::vec3(1, 0, 0)));
		mRotation = glm::normalize(glm::rotate(mRotation, eulerAngles.y, glm::vec3(0, 1, 0)));
		mRotation = glm::normalize(glm::rotate(mRotation, eulerAngles.z, glm::vec3(0, 0, 1.0f)));
	}

	void Vob::setDeletable(bool deletable)
	{
		mIsDeletable = deletable;
	}

	void Vob::setMeshRootNode(SceneNode* node)
	{
		if (mMeshRootNode) delete mMeshRootNode;
		mMeshRootNode = node;
	}

	void Vob::setOrientation(const glm::vec3& eulerAngles)
	{
		const auto rotX = glm::normalize(glm::rotate(glm::quat(), eulerAngles.x, glm::vec3(1, 0, 0)));
		const auto rotY = glm::normalize(glm::rotate(glm::quat(), eulerAngles.y, glm::vec3(0, 1, 0)));
		const auto rotZ = glm::normalize(glm::rotate(glm::quat(), eulerAngles.z, glm::vec3(0, 0, 1.0f)));
		mRotation = rotZ * rotY * rotX;
	}

	void Vob::setRotation(const glm::mat4& rotation)
	{
		mRotation = rotation;
	}

	void Vob::setRotation(const glm::quat& rotation)
	{
		mRotation = rotation;
	}

	void Vob::setPosition(const glm::vec3& position)
	{
		mPosition = position;
	}

	void Vob::setScale(const glm::vec3& scale)
	{
		mScale = scale;
	}

	void Vob::setSelectable(bool selectable)
	{
		mSelectable = selectable;
	}

	void Vob::setTrafo(const glm::mat4& mat)
	{
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(mat, mScale, mRotation, mPosition, skew, perspective);
	}

	void Vob::updateTrafo(bool resetPrevWorldTrafo)
	{
		if (!mMeshRootNode) return;

		const auto temp = glm::mat4();
		const auto rotation = toMat4(mRotation);
		const auto scaleMat = scale(temp, mScale);
		const auto transMat = translate(temp, mPosition);
		mMeshRootNode->setLocalTrafo(transMat * rotation * scaleMat);
		mMeshRootNode->updateWorldTrafoHierarchy(resetPrevWorldTrafo);
		recalculateBoundingBox();
	}

	void Vob::recalculateBoundingBox()
	{
		mBoundingBox = { glm::vec3(FLT_MAX), glm::vec3(-FLT_MAX) };

		if (!mMeshRootNode)
		{
			return;
		}

		std::queue<SceneNode*> nodes;
		nodes.push(mMeshRootNode);

		while (!nodes.empty())
		{
			auto* node = nodes.front();
			nodes.pop();

			const auto& children = node->getChildren();
			for (auto& child : children)
				nodes.push(child);


			const auto* mesh = node->getMesh();
			if (!mesh) continue;
			mBoundingBox = maxAABB(mBoundingBox, mesh->getAABB());
		}
	}
	MeshOwningVob::MeshOwningVob(std::unique_ptr<MeshContainer> container) : 
		Vob(nullptr)
	{
		setMeshContainer(std::move(container));
	}
	void MeshOwningVob::setMeshContainer(std::unique_ptr<MeshContainer> container)
	{
		mContainer = std::move(container);
		if (mContainer)
			setMeshRootNode(mContainer->createNodeHierarchyUnsafe());
	}
	MeshContainer* MeshOwningVob::getMesh() const
	{
		return mContainer.get();
	}
	MeshOwningVob::~MeshOwningVob() = default;


	RiggedVob::RiggedVob(SceneNode* meshRootNode) : Vob(meshRootNode), mAnimationTime(0.0f)
	{
		auto* skinnedMesh = dynamic_cast<SkinnedMesh*>(findFirstLegalMesh(meshRootNode));

		if (skinnedMesh == nullptr) {
			throw_with_trace(std::invalid_argument("RiggedVob::RiggedVob: meshRootNode is expected to contain at least one SkinnedMesh instance!"));
		}

		auto id = skinnedMesh->getRigID();
		mRig = AnimationManager::get()->getBySID(SID(id));

	}
	
	RiggedVob::~RiggedVob() = default;
	
	void RiggedVob::frameUpdate(float frameTime)
	{
		if (mActiveAnimation == nullptr) return;
		
		updateTime(frameTime);
		
		auto minMaxs = mActiveAnimation->calcMinMaxKeyFrames(mAnimationTime);
		auto interpolatedTrafos = BoneAnimation::calcInterpolatedTrafo(minMaxs, mAnimationTime);
		minMaxs.clear();

		mBoneTrafos = BoneAnimation::calcBoneTrafo(interpolatedTrafos);
		interpolatedTrafos.clear();

		mActiveAnimation->applyParentHierarchyTrafos(mBoneTrafos);
		mActiveAnimation->applyLocalToBoneSpaceTrafos(mBoneTrafos);
	}

	const std::vector<glm::mat4>& RiggedVob::getBoneTrafos() const
	{
		return mBoneTrafos;
	}

	void RiggedVob::setActiveAnimation(const std::string& animationName)
	{
		auto* ani = AnimationManager::get()->getBoneAnimation(SID(animationName));
		setActiveAnimation(ani);
	}

	void RiggedVob::setActiveAnimation(const BoneAnimation* animation)
	{
		// Ensure that the rig of the animation matches the rig of the skinned mesh
		if (animation->getRig() != mRig) {
			throw_with_trace(std::invalid_argument("RiggedVob::setActiveAnimation: Rig of new animation doesn't match the rig of the skinned mesh!"));
		}

		mActiveAnimation = animation;
		mAnimationTime = 0.0f;
	}

	void RiggedVob::setRepeatType(AnimationRepeatType type)
	{
		mRepeatType = type;
	}

	Mesh* RiggedVob::findFirstLegalMesh(SceneNode* node)
	{
		auto* mesh = node->getMesh();

		if (mesh) return mesh;

		for (auto* child : node->getChildren()) {
			mesh = findFirstLegalMesh(child);
			if (mesh) return mesh;
		}

		return nullptr;
	}

	void RiggedVob::updateTime(float frameTime)
	{
		mAnimationTime += frameTime;
		float duration = mActiveAnimation->getDuration();

		if (mAnimationTime <= duration) return;

		switch (mRepeatType) {
		case AnimationRepeatType::END:
				mAnimationTime = duration;
				break;
		case AnimationRepeatType::LOOP: {
			int multiplicatives = static_cast<int>(mAnimationTime / duration);
			mAnimationTime = mAnimationTime - multiplicatives * duration;
			break;
		}
		}
	}
}