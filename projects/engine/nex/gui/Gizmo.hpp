#pragma once
#include <memory>


namespace nex
{
	class Ray;
	class Scene;
	class SceneNode;
	class Mesh;
	class StaticMeshContainer;
	class Technique;
	class TransformPass;
}

namespace nex::gui
{
	class Gizmo
	{
	public:

		enum class Axis
		{
			X,
			Y,
			Z
		};

		struct Active
		{
			bool isActive;
			Axis axis;
		};
		
		Gizmo();
		virtual ~Gizmo();

		/**
		 * Traverses a scene and picks a scene node by a screen ray.
		 * If the ray intersects no node nullptr will be returned.
		 */
		Active isActive(const Ray& screenRayWorld);

		SceneNode* getGizmoNode();

	private:
		static std::unique_ptr<Mesh> createTranslationMesh();

		std::unique_ptr<StaticMeshContainer> mTranslationMesh;
		std::unique_ptr<TransformPass> mGizmoPass;
		std::unique_ptr<Technique> mGizmoTechnique;
		std::unique_ptr<Scene> mNodeGeneratorScene;

		SceneNode* mTranslationGizmoNode;
	};
}
