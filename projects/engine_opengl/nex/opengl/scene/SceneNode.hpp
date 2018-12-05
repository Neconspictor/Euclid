#pragma once
#include <vector>
#include <nex/opengl/model/Vob.hpp>
#include <nex/opengl/drawing/ModelDrawerGL.hpp>

namespace nex
{
	class RendererOpenGL;

	class SceneNode
	{
	public:
		explicit SceneNode();

		SceneNode(const SceneNode& copy);
		SceneNode(SceneNode&& copy);
		SceneNode& operator=(const SceneNode& copy);
		SceneNode&& operator=(SceneNode&& copy);

		virtual ~SceneNode() = default;

		void addChild(SceneNode* child);
		void removeChild(SceneNode* child);

		void init(ModelManagerGL* modelManager);

		void update(float frameTime);

		// public for convenient editing!
		SceneNode* parent;
		Vob* vob;
		glm::mat4 worldTrafo;
		glm::mat4 localTrafo;
		std::vector<SceneNode*> childs;
		DrawingTypes drawingType;
		int instanceCount;
	};
}