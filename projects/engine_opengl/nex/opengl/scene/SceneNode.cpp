#include <nex/opengl/scene/SceneNode.hpp>

SceneNode::SceneNode(): parent(nullptr), vob(nullptr), drawingType(DrawingTypes::SOLID), instanceCount(0)
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

SceneNode::~SceneNode()
{
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

std::vector<SceneNode*>::iterator SceneNode::getChildsBegin()
{
	return childs.begin();
}

std::vector<SceneNode*>::iterator SceneNode::getChildsEnd()
{
	return childs.end();
}

void SceneNode::init(ModelManagerGL* modelManager)
{
	for (auto it = childs.begin(); it != childs.end(); ++it)
		(*it)->init(modelManager);

	if (!vob) return;

	vob->init(modelManager);
}

void SceneNode::update(float frameTime)
{
	glm::mat4 localTrafo;
	if (vob)
	{
		vob->calcTrafo();
		localTrafo = vob->getTrafo();
	} else
		localTrafo = glm::mat4();

	if (parent)
		worldTrafo = parent->getWorldTrafo() * localTrafo;
	else
		worldTrafo = localTrafo;

	for (auto it = childs.begin(); it != childs.end(); ++it)
		(*it)->update(frameTime);
}

void SceneNode::draw(RendererOpenGL* renderer, ModelDrawerGL* drawer, const glm::mat4& projection, 
	const glm::mat4& view, ShaderType forcedShader)
{
	for (auto it = childs.begin(); it != childs.end(); ++it)
		(*it)->draw(renderer, drawer, projection, view, forcedShader);

	if (!vob) return;

	ShaderType type = vob->getMaterialShaderType();
	if (forcedShader != ShaderType::Unknown)
		type = forcedShader;

	//vob->calcTrafo();
	TransformData data = { &projection, &view, &vob->getTrafo() };
	if (drawingType == DrawingTypes::SOLID)
	{
		drawer->draw(vob, type, data);
	}
	else if (drawingType == DrawingTypes::INSTANCED)
	{
		nex::Logger("SceneNode")(nex::Warning) << "Instanced Drawing type currently not supported";
		//drawer->drawInstanced(vob, type, data, instanceCount);
	}
}

Vob* SceneNode::getVob() const
{
	return vob;
}

void SceneNode::setVob(Vob* vob)
{
	this->vob = vob;
}

void SceneNode::setDrawingType(DrawingTypes type)
{
	drawingType = type;
}

const glm::mat4& SceneNode::getWorldTrafo() const
{
	return worldTrafo;
}

const glm::mat4& SceneNode::getLocalTrafo() const
{
	return localTrafo;
}

void SceneNode::setInstanceCount(int count)
{
	instanceCount = count;
}