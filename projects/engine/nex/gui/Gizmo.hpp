#pragma once
#include <memory>
#include "nex/math/Ray.hpp"
#include "nex/Input.hpp"
#include "nex/math/Plane.hpp"
#include "nex/math/Torus.hpp"
#include <nex/math/Circle.hpp>


namespace nex
{
	struct Torus;
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
			bool isActive = false;
			Axis axis = Axis::INVALID;
			glm::vec3 axisVec = glm::vec3(0.0f);
			glm::vec3 originalPosition = glm::vec3(0.0f);
			Ray originalRay = Ray(glm::vec3(0.0f), glm::vec3(1.0f));
			Torus torus = Torus();
			float range = 0.0f;
			glm::quat originalRotation = glm::quat(1, 0, 0, 0);
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
		void activate(const Ray& screenRayWorld, float cameraViewFieldRange, SceneNode* node);

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

		bool isHovering(const Ray& screenRayWorld, float cameraViewFieldRange, Active& active) const;

		bool isVisible()const;

		/**
		 * Provides a scene node that can be used to render the gizmo.
		 */
		SceneNode* getGizmoNode();

		void transform(const Ray& screenRayWorld, SceneNode& node, const Camera& camera, const MouseOffset& frameData);
		void deactivate();

		void setMode(Mode mode);

		void show(Scene& scene, const SceneNode& node);
		void hide(Scene& scene);

	private:

		struct Data
		{
			Ray::RayDistance result;
			glm::vec3 axisVector;
			Axis axis;
		};

		class GizmoPass;

		int compare(const Data& first, const Data& second) const;
		void initSceneNode(SceneNode*& node, StaticMeshContainer* container, const char* debugName);
		bool isHoveringRotate(const Ray& screenRayWorld, const float cameraViewFieldRange, Active& active) const;

		/**
		 * @param multiplierOut : The multiplier of the ray plane intersection test, if the ray intersects the min-max circle geometry.					  
		 */
		bool checkNearPlaneCircle(const Plane::RayIntersection& testResult,
								const Ray& ray,
								const glm::vec3& circleOrigin, 
								float minRadius, float maxRadius, float& multiplierOut) const;

		static bool hitsTorus(const Torus& torus, const glm::vec3& orientation, const glm::vec3& origin, const Ray& ray,
			nex::Torus::RayIntersection& intersectionTest);

		void fillActivationState(Active& active, bool isActive, Axis axis, const glm::vec3& position, const Ray& ray, float range) const;

		void transformRotate(const Ray& ray, SceneNode& node, const Camera& camera);

		StaticMeshContainer* loadRotationGizmo();
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
		float mRotationAcc;
		glm::vec3 mRotationVecLast;
		Mode mMode;
		bool mVisible;

		SceneNode* mModifiedNode = nullptr;
	};
}