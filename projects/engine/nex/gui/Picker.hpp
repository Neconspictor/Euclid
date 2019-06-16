#pragma once
#include <memory>


namespace nex
{
	class Ray;
	class Scene;
	class SceneNode;
	class Mesh;
	class StaticMeshContainer;
	class SimpleColorPass;
	class Technique;
}

namespace nex::gui
{
	class Picker
	{
	public:

		Picker();
		virtual ~Picker();

		/**
		 * Traverses a scene and picks a scene node by a screen ray.
		 * If the ray intersects no node nullptr will be returned.
		 */
		SceneNode* pick(Scene& scene, const Ray& screenRayWorld);

		SceneNode* getPicked();

		/**
		 * Updates the world transformation matrix of the bounding box if a scene node is currently selected.
		 */
		void updateBoundingBoxTrafo();

	private:

		static std::unique_ptr<Mesh> createBoundingBoxMesh();
		static std::unique_ptr<Mesh> createBoundingBoxLineMesh();

		static std::unique_ptr<Mesh> createLineMesh();

		std::unique_ptr<StaticMeshContainer> mBoundingBoxMesh;
		//std::unique_ptr<StaticMeshContainer> mLineMesh;
		std::unique_ptr<SimpleColorPass> mSimpleColorPass;
		std::unique_ptr<Technique> mSimpleColorTechnique;
		std::unique_ptr<Scene> mNodeGeneratorScene;

		SceneNode* mBoundingBoxNode;
		//SceneNode* mLineNode;
		SceneNode* mSelectedNode;
	};
}