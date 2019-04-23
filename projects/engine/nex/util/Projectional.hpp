#pragma once

#include <glm/glm.hpp>
#include "nex/common/Log.hpp"

namespace nex
{

	struct Frustum
	{
		glm::vec3 farLeftBottom;
		glm::vec3 farLeftTop;
		glm::vec3 farRightBottom;
		glm::vec3 farRightTop;

		glm::vec3 nearLeftBottom;
		glm::vec3 nearLeftTop;
		glm::vec3 nearRightBottom;
		glm::vec3 nearRightTop;
	};

	/**
	 * Defines a coordinate system by a position and a up and look vector. The right vector is indirectly defined by the cross product of up and look vector:
	 * right = cross(look, up)
	 */
	struct PULCoordinateSystem
	{
		glm::vec3 position = {0,0,0};
		glm::vec3 up = {0,1,0};
		glm::vec3 look = {0,0,-1};
	};

	/**
	 * A base class for perspective cameras.
	 */
	class PerspectiveCamera
	{
	public:
		explicit PerspectiveCamera(float aspectRatio = 16.0f / 9.0f,
			float fovY = 45.0f, // the vertical field of view (in degrees)
			float nearDistance = 0.1f, // the distance to the near clipping plane
			float farDistance = 100.0f, // the distance to the far clipping plane
			PULCoordinateSystem coordinateSystem = PULCoordinateSystem()
		);

		/**
		 * Recalculates the camera's view frustum (in view space).
		 */
		void calcFrustum();

		/**
		 * Recalculates the view space matrix of the camera.
		 */
		void calcView();

		/**
		 * Provides the aspect ratio of the camera's canvas.
		 */
		float getAspectRatio() const;

		/**
		 * Provides the distance to the far clipping plane.
		 */
		float getFarDistance() const;

		/**
		 * Provides the vertical field of view angle of the camera.
		 */
		float getFovY() const;

		/**
		 * Provides the current calculated view frustum (in view space).
		 */
		const Frustum& getFrustum();

		/**
		 * Calculates the view frustum in world space.
		 */
		Frustum calcFrustumWorld() const;

		/**
		 * Provides the current look direction of the camera.
		 */
		const glm::vec3& getLook() const;

		/**
		 * Provides the perspective projection matrix of the camera.
		 */
		const glm::mat4& getProjectionMatrix() const;

		/**
		 * Provides the position of the camera.
		 */
		const glm::vec3& getPosition() const;

		/**
		 * Provides the distance to the near clipping plane.
		 */
		float getNearDistance() const;

		/**
		 * Provides the right vector, which is defined by the cross product of look and up vector:
		 * right = cross(look, up)
		 */
		const glm::vec3& getRight() const;

		/**
		 * Provides the up vector of the camera's coordinates system.
		 */
		const glm::vec3& getUp() const;

		/**
		 * Provides the last calculated view matrix.
		 */
		const glm::mat4& getView() const;

		/**
		 * Calculate viewspace z from a distance to camera
		 */
		static float getViewSpaceZfromDistance(float distance);

		/**
		 * Orients the camera so that it looks to a given location.
		 */
		void lookAt(glm::vec3 location);

		/**
		 * Sets the aspect ratio of the camera's canvas.
		 * NOTE: ratio as to be greater 0, otherwise a runtime_error will be thrown!
		 */
		void setAspectRatio(float ratio);

		/**
		 * Sets the vertical field of view angle (measured in degrees). 
		 * The provided angle will be clamped to the range [0, 180]
		 */
		void setFovY(float fov);

		/**
		 * Sets the distance to the near clipping plane.
		 */
		void setNearDistance(float nearPlane);

		/**
		 * Sets the distance to the far clipping plane.
		 */
		void setFarDistance(float farPlane);

		/**
		 * Sets the look direction of the camera.
		 * NOTE: Has to be a vector that isn't a null vector. So it's length has to be > 0
		*/
		void setLook(glm::vec3 look);

		/**
		 * Sets the position of the camera.
		 */
		void setPosition(glm::vec3 position);

		/**
		 * Sets the up vector of the camera's coordinates system.
		 */
		void setUp(glm::vec3 up);

		/**
		 * If there are any pending changes, the view matrix etc. are recalculated. If updateAlways is true, 
		 * the recalculations take place even than there are no pending changes.
		 */
		void update(bool updateAlways = false);



	private:

		static void assertValidVector(const glm::vec3&);

		float mAspectRatio;
		PULCoordinateSystem mCoordSystem;
		float mFovY;
		Frustum mFrustum;
		nex::Logger mLogger;
		glm::mat4 mProjection;
		glm::vec3 mTargetPosition;
		bool mRevalidate;
		glm::vec3 mRight;
		glm::mat4 mView;
		float mFarDistance;
		float mNearDistance;
	};
}