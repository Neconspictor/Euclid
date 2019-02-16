#include <nex/SceneNode.hpp>

namespace nex
{
	SceneNode::SceneNode() : parent(nullptr), vob(nullptr), drawingType(DrawingTypes::SOLID), instanceCount(0)
	{
	}

	SceneNode::SceneNode(const SceneNode& copy) :
		parent(copy.parent), vob(copy.vob), worldTrafo(copy.worldTrafo), localTrafo(copy.localTrafo),
		childs(copy.childs), drawingType(copy.drawingType), instanceCount(copy.instanceCount)
	{

	}

	SceneNode::SceneNode(SceneNode&& copy) :
		parent(copy.parent), vob(copy.vob), worldTrafo(copy.worldTrafo), localTrafo(copy.localTrafo),
		childs(copy.childs), drawingType(copy.drawingType), instanceCount(copy.instanceCount)
	{
		copy.parent = nullptr;
		copy.vob = nullptr;
		copy.childs.clear();
	}

	SceneNode& SceneNode::operator=(const SceneNode& copy)
	{
		if (this == &copy) return *this;
		parent = copy.parent;
		vob = copy.vob;
		worldTrafo = copy.worldTrafo;
		localTrafo = copy.localTrafo;
		childs = copy.childs;
		drawingType = copy.drawingType;
		instanceCount = copy.instanceCount;
		return *this;
	}

	SceneNode&& SceneNode::operator=(SceneNode&& copy)
	{
		if (this == &copy) return std::move(*this);
		parent = copy.parent;
		vob = copy.vob;
		worldTrafo = copy.worldTrafo;
		localTrafo = copy.localTrafo;
		childs = move(copy.childs);
		drawingType = copy.drawingType;
		instanceCount = copy.instanceCount;

		copy.parent = nullptr;
		copy.vob = nullptr;
		copy.childs.clear();

		return std::move(*this);
	}

	void SceneNode::addChild(SceneNode* child)
	{
		auto it = find(childs.begin(), childs.end(), child);
		if (it == childs.end())
		{
			childs.push_back(child);
			child->parent = this;
		}
	}

	void SceneNode::removeChild(SceneNode* child)
	{
		auto it = find(childs.begin(), childs.end(), child);
		if (it != childs.end())
		{
			(*it)->parent = nullptr;
			childs.erase(it);
		}
	}

	void SceneNode::init()
	{
		for (auto it = childs.begin(); it != childs.end(); ++it)
			(*it)->init();

		if (!vob) return;

		vob->init();
	}

	void SceneNode::update(float frameTime)
	{
		glm::mat4 localTrafo;
		if (vob)
		{
			vob->calcTrafo();
			localTrafo = vob->getTrafo();
		}
		else
			localTrafo = glm::mat4();

		if (parent)
			worldTrafo = parent->worldTrafo * localTrafo;
		else
			worldTrafo = localTrafo;

		for (auto it = childs.begin(); it != childs.end(); ++it)
			(*it)->update(frameTime);
	}
}