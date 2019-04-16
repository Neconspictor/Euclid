#include <nex/Scene.hpp>
#include <queue>
#include <glm/gtc/matrix_transform.hpp>

namespace nex
{
	SceneNode::SceneNode() : mMesh(nullptr), mMaterial(nullptr),
		mParent(nullptr)
	{
	}

	void SceneNode::addChild(SceneNode* node)
	{
		if (mChildren.insert(node).first != mChildren.end())
		{
			node->mParent = this;
		}
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

	const glm::mat4& SceneNode::getLocalTrafo() const
	{
		return mLocalTrafo;
	}

	const glm::mat4& SceneNode::getWorldTrafo() const
	{
		return mWorldTrafo;
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

	void SceneNode::setLocalTrafo(const glm::mat4& trafo)
	{
		mLocalTrafo = trafo;
	}

	void SceneNode::setWorldTrafo(const glm::mat4& trafo)
	{
		mWorldTrafo = trafo;
	}

	void SceneNode::updateChildrenWorldTrafos()
	{
		//assume that the world trafo of the current node is up to date (needs no update from the parent)
		auto* backup = mParent;
		mParent = nullptr;
		updateWorldTrafoHierarchy();
		mParent = backup;
	}

	void SceneNode::updateWorldTrafoHierarchy()
	{
		std::queue<SceneNode*> queue;
		queue.push(this);

		while (!queue.empty())
		{
			auto* node = queue.front();
			queue.pop();

			node->updateWorldTrafo();

			auto children = node->getChildren();

			for (auto it = children.begin; it != children.end; ++it)
				queue.push(*it);
		}
	}

	void SceneNode::setPositionLocal(glm::vec3 position)
	{
		mLocalTrafo[3] = glm::vec4(position, 1.0f);
	}

	void SceneNode::updateWorldTrafo()
	{
		if (mParent)
		{
			mWorldTrafo = mParent->mWorldTrafo * mLocalTrafo;
		}
		else
		{
			mWorldTrafo = mLocalTrafo;
		}
	}

	Scene::Scene() : mRoot(std::make_unique<SceneNode>())
	{
	}

	SceneNode* Scene::createNode(SceneNode* parent)
	{
		mNodes.emplace_back(std::make_unique<SceneNode>());
		auto* node = mNodes.back().get();
		node->setParent(parent);
		if (parent) parent->addChild(node);

		return node;
	}

	SceneNode* Scene::getRoot() const
	{
		return mRoot.get();
	}

	void Scene::clear()
	{
		mRoot->clear();
		mNodes.clear();
	}
}