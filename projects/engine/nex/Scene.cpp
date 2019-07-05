#include <nex/Scene.hpp>
#include <queue>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <nex/math/Math.hpp>
#include <nex/mesh/Mesh.hpp>

namespace nex
{
	SceneNode::SceneNode() : mMesh(nullptr), mMaterial(nullptr),
		mParent(nullptr)
	{
	}

	void SceneNode::addChild(SceneNode* node)
	{
		mChildren.push_back(node);
		node->mParent = this;
	}

	void SceneNode::clear()
	{
		mChildren.clear();
	}

	SceneNode::Children SceneNode::getChildren() const
	{
		return  { mChildren.begin(), mChildren.end() };
	}

	Mesh* SceneNode::getMesh() const
	{
		return mMesh;
	}

	Material* SceneNode::getMaterial() const
	{
		return mMaterial;
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

	void SceneNode::removeChild(SceneNode* node)
	{
		auto it = std::find(mChildren.begin(), mChildren.end(), node);
		if (it != mChildren.end())
		{
			(*it)->mParent = nullptr;
			mChildren.erase(it);
		}
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

			auto children = node->getChildren();

			for (auto it = children.begin; it != children.end; ++it)
				queue.push(*it);
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

	Scene::Scene()
	{
	}

	UniqueLock Scene::acquireLock() const {
		return UniqueLock(mMutex);
	}

	void Scene::addActiveVobUnsafe(Vob* vob)
	{
		mActiveVobs.insert(vob);
	}

	void Scene::removeActiveVobUnsafe(Vob* vob)
	{
		mActiveVobs.erase(vob);
	}

	SceneNode* Scene::createNodeUnsafe(SceneNode* parent)
	{
		mNodes.emplace_back(std::make_unique<SceneNode>());
		auto* node = mNodes.back().get();
		node->setParent(parent);
		if (parent) parent->addChild(node);

		return node;
	}

	Vob* Scene::addVobUnsafe(std::unique_ptr<Vob> vob, bool setActive)
	{
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

	void Scene::clearUnsafe()
	{
		mNodes.clear();
		mActiveVobs.clear();
		mVobStore.clear();
	}

	const std::unordered_set<Vob*>& Scene::getActiveVobsUnsafe() const
	{
		return mActiveVobs;
	}

	void Scene::updateWorldTrafoHierarchyUnsafe(bool resetPrevWorldTrafo)
	{
		for (auto& vob : mActiveVobs)
			vob->updateTrafo(resetPrevWorldTrafo);
	}


	Vob::Vob(SceneNode* meshRootNode) : mMeshRootNode(meshRootNode), mPosition(0.0f), mRotation(glm::quat()), mScale(1.0f), mSelectable(true)
	{

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

	void Vob::setMeshRootNode(SceneNode* node)
	{
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

			const auto children = node->getChildren();
			for (auto it = children.begin; it != children.end; ++it)
				nodes.push(*it);


			const auto* mesh = node->getMesh();
			if (!mesh) continue;
			mBoundingBox = maxAABB(mBoundingBox, mesh->getAABB());
		}
	}
}