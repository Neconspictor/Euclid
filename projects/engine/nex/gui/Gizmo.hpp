#pragma once
#include <memory>
#include "nex/math/Ray.hpp"


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
			X = 0,
			Y = 1,
			Z = 2,
			INVALID = 3,
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
		Active isActive(const Ray& screenRayWorld, float cameraViewFieldRange);

		/**
		 * Configures internal shaders so that the specified axis will be highlighted 
		 * on the next render call.
		 * Note: The current bound shader might change!
		 * 
		 * @param axis : The axis that should be highlighted. 
		 *				 If axis is Axis::INVALID no axis will be highlighted (default state).
		 */
		void highlightAxis(Axis axis);

		/**
		 * Provides a scene node that can be used to render the gizmo.
		 */
		SceneNode* getGizmoNode();

	private:

		struct Data
		{
			Ray::RayRayDistance result;
			Axis axis;
		};

		class TranslationGizmoPass;

		int compare(const Data& first, const Data& second) const;
		static std::unique_ptr<Mesh> createTranslationMesh();


		std::unique_ptr<StaticMeshContainer> mTranslationMesh;
		std::unique_ptr<TranslationGizmoPass> mTranslationGizmoPass;
		std::unique_ptr<Technique> mGizmoTechnique;
		std::unique_ptr<Scene> mNodeGeneratorScene;

		SceneNode* mTranslationGizmoNode;
	};
}
