#include <nex/SceneNode.hpp>

namespace nex
{
	SceneNode::SceneNode() : mParent(nullptr), mVob(nullptr), mDrawingType(DrawingTypes::SOLID), mInstanceCount(0)
	{
	}

	void SceneNode::addChild(SceneNode* child)
	{
		auto it = find(mChilds.begin(), mChilds.end(), child);
		if (it == mChilds.end())
		{
			mChilds.push_back(child);
			child->mParent = this;
		}
	}

	void SceneNode::removeChild(SceneNode* child)
	{
		auto it = find(mChilds.begin(), mChilds.end(), child);
		if (it != mChilds.end())
		{
			(*it)->mParent = nullptr;
			mChilds.erase(it);
		}
	}

	void SceneNode::init()
	{
		for (auto it = mChilds.begin(); it != mChilds.end(); ++it)
			(*it)->init();

		if (!mVob) return;

		mVob->init();
	}

	void SceneNode::update(float frameTime)
	{
		glm::mat4 localTrafo;
		if (mVob)
		{
			mVob->calcTrafo();
			localTrafo = mVob->getTrafo();
		}
		else
			localTrafo = glm::mat4();

		if (mParent)
			mWorldTrafo = mParent->mWorldTrafo * localTrafo;
		else
			mWorldTrafo = localTrafo;

		for (auto it = mChilds.begin(); it != mChilds.end(); ++it)
			(*it)->update(frameTime);
	}
}