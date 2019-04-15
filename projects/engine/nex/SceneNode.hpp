#pragma once
#include <vector>
#include <nex/mesh/Vob.hpp>
#include <nex/drawing/StaticMeshDrawer.hpp>

namespace nex
{
	class StaticMeshManager;
	class RenderBackend;

	class SceneNode
	{
	public:
		SceneNode();

		void addChild(SceneNode* child);
		void removeChild(SceneNode* child);

		void init();

		void update(float frameTime);

		// public for convenient editing!
		SceneNode* mParent;
		Vob* mVob;
		glm::mat4 mWorldTrafo;
		glm::mat4 mLocalTrafo;
		std::vector<SceneNode*> mChilds;
		DrawingTypes mDrawingType;
		int mInstanceCount;
	};
}