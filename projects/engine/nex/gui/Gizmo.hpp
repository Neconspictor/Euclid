#pragma once
#include <memory>
#include "nex/math/Ray.hpp"
#include "nex/Input.hpp"


namespace nex
{
	class Ray;
	class Scene;
	class SceneNode;
	class Mesh;
	class StaticMeshContainer;
	class Technique;
	class TransformPass;
	class Camera;
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
			glm::vec3 originalPosition;
		};

		enum class Mode
		{
			ROTATE,
			SCALE,
			TRANSLATE,
		};
		
		Gizmo(Mode mode = Mode::TRANSLATE);
		virtual ~Gizmo();

		void update(const nex::Camera& camera);

		/**
		 * Conditionally activates the gizmo if the screen ray traverses near one of the gizmo's axis.
		 */
		void activate(const Ray& screenRayWorld, float cameraViewFieldRange);

		Mode getMode()const;

		/**
		 * Provides the current state of the gizmo.
		 */
		const Active& getState()const;

		/**
		 * Configures internal shaders so that the specified axis will be highlighted 
		 * on the next render call.
		 * Note: The current bound shader might change!
		 * 
		 * @param axis : The axis that should be highlighted. 
		 *				 If axis is Axis::INVALID no axis will be highlighted (default state).
		 */
		void highlightAxis(Axis axis);

		bool isHovering(const Ray& screenRayWorld, float cameraViewFieldRange, Active* active = nullptr) const;

		/**
		 * Provides a scene node that can be used to render the gizmo.
		 */
		SceneNode* getGizmoNode();

		void transform(const Ray& screenRayWorld, SceneNode& node, const MouseOffset& frameData);
		void deactivate();

		void setMode(Mode mode);

	private:

		struct Data
		{
			Ray::RayRayDistance result;
			glm::vec3 axisVector;
			Axis axis;
		};

		class GizmoPass;

		int compare(const Data& first, const Data& second) const;
		void initSceneNode(SceneNode*& node, StaticMeshContainer* container, const char* debugName);
		//static std::unique_ptr<Mesh> createTranslationMesh();
		StaticMeshContainer* loadTranslationGizmo();
		StaticMeshContainer* loadScaleGizmo();

		StaticMeshContainer* mRotationMesh;
		StaticMeshContainer* mScaleMesh;
		StaticMeshContainer* mTranslationMesh;
		
		std::unique_ptr<GizmoPass> mGizmoPass;
		std::unique_ptr<Technique> mGizmoTechnique;
		std::unique_ptr<Scene> mNodeGeneratorScene;

		SceneNode* mRotationGizmoNode;
		SceneNode* mScaleGizmoNode;
		SceneNode* mTranslationGizmoNode;

		SceneNode* mActiveGizmoNode;

		Active mActivationState;
		float mLastFrameMultiplier;
		Mode mMode;
	};
}