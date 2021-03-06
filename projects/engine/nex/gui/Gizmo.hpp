#pragma once
#include <memory>
#include "nex/math/Ray.hpp"
#include "nex/math/Plane.hpp"
#include "nex/math/Torus.hpp"
#include <nex/gui/Drawable.hpp>


namespace nex
{
	struct MouseOffset;
	struct Torus;
	class Ray;
	class Vob;
	class Mesh;
	class MeshGroup;
	class TransformShader;
	class Camera;
	class AbstractMaterialLoader;
	class AbstractMeshLoader;
	class Scene;
}

namespace nex::gui
{
	class Gizmo
	{
	public:

		enum class Axis
		{
			INVALID = 0,
			X = 1,
			Y = 2,
			Z = 4,
		};

		struct Active
		{
			bool isActive = false;
			Axis axis = Axis::INVALID;
			glm::vec3 axisVec = glm::vec3(0.0f);
			glm::vec3 orthoAxisVec = glm::vec3(0.0f);
			glm::vec3 originalPosition = glm::vec3(0.0f);

			float range = 0.0f;
			glm::quat originalRotation = glm::quat(1, 0, 0, 0);
			float startRotationAngle = 0.0f;
			
			bool scaleUniform = false;

			static float calcRange(const Ray& ray, const glm::vec3& position, const Camera& camera);
		};

		enum class Mode
		{
			ROTATE,
			SCALE,
			TRANSLATE,
		};

		Gizmo(Mode mode = Mode::TRANSLATE);
		virtual ~Gizmo();

		void syncTransformation();
		void update(const nex::Camera& camera, Vob* vob);

		/**
		 * Conditionally activates the gizmo if the screen ray traverses near one of the gizmo's axis.
		 */
		void activate(const Ray& screenRayWorld, const Camera& camera, bool scaleUniform);

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
		void highlightAxis(unsigned axis);

		bool isHovering(const Ray& screenRayWorld, const Camera& camera);
		bool isVisible()const;

		/**
		 * Provides a scene node that can be used to render the gizmo.
		 */
		Vob* getGizmoNode();

		void transform(const Ray& screenRayWorld, const Camera& camera, const MouseOffset& frameData);
		void deactivate();

		void setMode(Mode mode);

		void show(Scene* scene);
		void hide();

		bool getShowGizmo() const;
		void setShowGizmo(bool show);

	private:

		struct Data
		{
			Ray::RayDistance result;
			glm::vec3 axisVector;
			Axis axis;
		};

		class Material;
		class GizmoPass;
		class MaterialLoader;

		int compare(const Data& first, const Data& second) const;

		float calcRotation(const Ray& ray, const glm::vec3& axis, const glm::vec3& orthoAxis, const Camera& camera) const;

		void initSceneNode(Vob* node, const char* debugName);

		bool isHovering(const Ray& screenRayWorld, const Camera& camera, bool fillActive);
		bool isHoveringRotate(const Ray& screenRayWorld, const Camera& camera, bool fillActive);

		/**
		 * @param multiplierOut : The multiplier of the ray plane intersection test, if the ray intersects the min-max circle geometry.
		 */
		bool checkNearPlaneCircle(const Plane::RayIntersection& testResult,
			const Ray& ray,
			const glm::vec3& circleOrigin,
			float minRadius, float maxRadius, float& multiplierOut) const;

		static bool hitsTorus(const Torus& torus, const glm::vec3& orientation, const glm::vec3& origin, const Ray& ray,
			nex::Torus::RayIntersection& intersectionTest);

		void fillActivationState(Active& active, bool isActive, Axis axis, const glm::vec3& position, const Ray& ray, const Camera& camera) const;

		void transformRotate(const Ray& ray, const Camera& camera);

		std::unique_ptr<GizmoPass> mGizmoPass;

		std::unique_ptr<MaterialLoader> mMaterialLoader;

		std::unique_ptr<Vob> mRotationGizmoNode;
		std::unique_ptr<Vob> mScaleGizmoNode;
		std::unique_ptr<Vob> mTranslationGizmoNode;

		Vob* mActiveGizmoVob;

		Active mActivationState;
		float mLastFrameMultiplier;
		Mode mMode;
		bool mVisible;

		Vob* mModifiedNode = nullptr;
		Scene* mScene = nullptr;
	};


	class Gizmo_View : public nex::gui::Drawable {
	public:
		Gizmo_View(Gizmo* gizmo);
	protected:

		void drawSelf() override;

		Gizmo* mGizmo;
	};
}