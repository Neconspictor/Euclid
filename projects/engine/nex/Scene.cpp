#include <nex/Scene.hpp>
#include <queue>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <nex/math/Math.hpp>
#include <nex/mesh/Mesh.hpp>

namespace nex
{
	SceneNode::SceneNode() : mMesh(nullptr), mMaterial(nullptr),
		mParent(nullptr), mPosition(0.0f), mRotation(glm::quat()), mScale(1.0f), mSelectable(true)
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

	const glm::vec3& SceneNode::getPosition() const
	{
		return mPosition;
	}

	const glm::quat& SceneNode::getRotation() const
	{
		return mRotation;
	}

	const glm::vec3& SceneNode::getScale() const
	{
		return mScale;
	}

	bool SceneNode::getSelectable() const
	{
		return mSelectable;
	}

	void SceneNode::setSelectable(bool selectable)
	{
		mSelectable = selectable;
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

	void SceneNode::setPosition(const glm::vec3& position)
	{
		mPosition = position;
	}

	void SceneNode::setRotation(const glm::mat4& rotation)
	{
		mRotation = rotation;
	}

	void SceneNode::setRotation(const glm::quat& rotation)
	{
		mRotation = rotation;
	}

	void SceneNode::setOrientation(const glm::vec3& eulerAngles)
	{
		auto rotX = glm::normalize(glm::rotate(glm::quat(), eulerAngles.x, glm::vec3(1, 0, 0)));
		auto rotY = glm::normalize(glm::rotate(glm::quat(), eulerAngles.y, glm::vec3(0, 1, 0)));
		auto rotZ = glm::normalize(glm::rotate(glm::quat(), eulerAngles.z, glm::vec3(0, 0, 1.0f)));
		mRotation = rotZ * rotY * rotX;
	}

	void SceneNode::rotateGlobal(const glm::vec3& axisWorld, float angle)
	{
		mRotation = glm::normalize(glm::rotate(mRotation, angle, inverse(mRotation) * axisWorld));
	}

	void SceneNode::rotateLocal(const glm::vec3& eulerAngles)
	{
		mRotation = glm::normalize(glm::rotate(mRotation, eulerAngles.x, glm::vec3(1, 0, 0)));
		mRotation = glm::normalize(glm::rotate(mRotation, eulerAngles.y, glm::vec3(0, 1, 0)));
		mRotation = glm::normalize(glm::rotate(mRotation, eulerAngles.z, glm::vec3(0, 0, 1.0f)));
	}

	void SceneNode::rotateGlobal(const glm::vec3& eulerAngles)
	{
		mRotation = glm::normalize(glm::rotate(mRotation, eulerAngles.x, inverse(mRotation) * glm::vec3(1, 0, 0)));
		mRotation = glm::normalize(glm::rotate(mRotation, eulerAngles.y, inverse(mRotation) * glm::vec3(0, 1, 0)));
		mRotation = glm::normalize(glm::rotate(mRotation, eulerAngles.z, inverse(mRotation) * glm::vec3(0, 0, 1.0f)));
	}

	void SceneNode::setScale(const glm::vec3 scale)
	{
		mScale = scale;
	}

	void SceneNode::updateWorldTrafo(bool resetPrevWorldTrafo)
	{

		mPrevWorldTrafo = mWorldTrafo;
		mWorldTrafo = glm::mat4(1.0f);

		auto scale = glm::scale(glm::mat4(), mScale);

		auto trans = glm::translate(glm::mat4(), mPosition);

		mWorldTrafo = trans * glm::toMat4(mRotation) * scale;

		if (mParent)
		{
			mWorldTrafo = mParent->mWorldTrafo * mWorldTrafo;
		}

		if (resetPrevWorldTrafo)
			mPrevWorldTrafo = mWorldTrafo;
	}

	Scene::Scene()
	{
	}

	void Scene::addRoot(SceneNode* node)
	{
		assert(node->getParent() == nullptr);
		mRoots.emplace_back(node);
	}

	void Scene::removeRoot(SceneNode* node)
	{
		mRoots.erase(std::remove(mRoots.begin(), mRoots.end(), node), mRoots.end());
	}


	SceneNode* Scene::createNode(SceneNode* parent)
	{
		mNodes.emplace_back(std::make_unique<SceneNode>());
		auto* node = mNodes.back().get();
		node->setParent(parent);
		if (parent) parent->addChild(node);

		return node;
	}

	void Scene::clear()
	{
		mRoots.clear();
		mNodes.clear();
	}

	void Scene::updateWorldTrafoHierarchy(bool resetPrevWorldTrafo)
	{
		for (auto& root : mRoots)
			root->updateWorldTrafoHierarchy(resetPrevWorldTrafo);
	}

	const std::vector<SceneNode*> Scene::getRoots() const
	{
		return mRoots;
	}
}