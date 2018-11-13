#pragma once
#include <vector>
#include <nex/opengl/model/Vob.hpp>
#include <nex/opengl/drawing/ModelDrawerGL.hpp>
#include <nex/opengl/shader/ShaderGL.hpp>

class RendererOpenGL;

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
	std::vector<SceneNode*>::iterator getChildsBegin();
	std::vector<SceneNode*>::iterator getChildsEnd();

	void init(ModelManagerGL* modelManager);

	void update(float frameTime);
	/**
	 * Draws this scene node and all its children using a specific shader.
	 */
	void draw(RendererOpenGL* renderer, ModelDrawerGL* drawer, const glm::mat4& projection,
		const glm::mat4& view, Shaders forcedShader = Shaders::Unknown);

	//virtual void draw(ModelDrawer* drawer);

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