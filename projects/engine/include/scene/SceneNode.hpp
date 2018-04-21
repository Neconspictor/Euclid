#pragma once
#include <vector>
#include <model/Vob.hpp>
#include <drawing/ModelDrawer.hpp>
#include <shader/Shader.hpp>
#include <renderer/Renderer3D.hpp>

class SceneNode
{
public:
	explicit SceneNode();

	SceneNode(const SceneNode& copy);
	SceneNode(SceneNode&& copy);
	SceneNode& operator=(const SceneNode& copy);
	SceneNode&& operator=(SceneNode&& copy);

	virtual ~SceneNode();

	void addChild(SceneNode* child);
	void removeChild(SceneNode* child);
	std::vector<SceneNode*>::const_iterator getChildsBegin() const;
	std::vector<SceneNode*>::const_iterator getChildsEnd() const;

	virtual void update(float frameTime);
	/**
	 * Draws this scene node and all its children using a specific shader.
	 */
	virtual void draw(Renderer3D* renderer, ModelDrawer* drawer, const glm::mat4& projection, 
		const glm::mat4& view, Shaders forcedShader = Shaders::Unknown);

	Vob* getVob() const;
	void setVob(Vob* vob);

	void setDrawingType(DrawingTypes type);

	const glm::mat4& getWorldTrafo() const;
	const glm::mat4& getLocalTrafo() const;
	void setInstanceCount(int count);
protected:
	SceneNode* parent;
	Vob* vob;
	glm::mat4 worldTrafo;
	glm::mat4 localTrafo;
	std::vector<SceneNode*> childs;
	DrawingTypes drawingType;
	int instanceCount;
};
