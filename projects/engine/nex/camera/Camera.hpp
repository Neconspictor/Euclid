#pragma once

#include <glm/glm.hpp>
#include "nex/common/Log.hpp"

namespace nex
{
	class Input;

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

	class Camera
	{
	public:
		Camera(float nearDistance = 0.1f, // the distance to the near clipping plane
			float farDistance = 100.0f, // the distance to the far clipping plane
			PULCoordinateSystem coordinateSystem = PULCoordinateSystem());

		Camera(glm::vec3 position, glm::vec3 look, glm::vec3 up);

		Camera(const Camera&) = default;
		Camera(Camera&&) = default;
		Camera& operator=(const Camera&) = default;
		Camera& operator=(Camera&&) = default;
		virtual ~Camera() = default;

		/**
		 * Applies changes for the current frame.
		 */
		virtual void frameUpdate(Input* input, float frameTime);

		/**
		 * Provides the distance to the far clipping plane.
		 */
		float getFarDistance() const;

		/**
		 * Provides the current calculated view frustum (in view space).
		 */
		const Frustum& getFrustum();

		/**
		 * Provides the speed of the camera.
		 */
		float getSpeed() const;

		/**
		 * Calculates the view frustum in world space.
		 */
		Frustum calcFrustumWorld() const;

		/**
		 * Provides the current look direction of the camera.
		 */
		const glm::vec3& getLook() const;

		/**
		 * Provides the distance to the near clipping plane.
		 */
		float getNearDistance() const;

		/**
		 * Provides the position of the camera.
		 */
		const glm::vec3& getPosition() const;

		/**
		 * Provides the perspective projection matrix of the camera.
		 */
		const glm::mat4& getProjectionMatrix() const;

		/**
		 * Provides the right vector, which is defined by the cross product of look and up vector:
		 * right = cross(look, up)
		 */
		const glm::vec3& getRight() const;

		/**
		 * Provides the target position of the camera.
		 */
		const glm::vec3& getTargetPosition() const;

		/**
		 * Provides the up vector of the camera's coordinates system.
		 */
		const glm::vec3& getUp() const;

		/**
		 * Provides the last calculated view matrix.
		 */
		const glm::mat4& getView() const;

		/**
		 * Provides the near and far plane in viewspace 
		 */
		glm::vec2 getNearFarPlaneViewSpace() const;

		/**
		 * Calculate viewspace z from a distance to camera
		 */
		static float getViewSpaceZfromDistance(float distance);

		/**
		 * Orients the camera so that it looks to a given location.
		 */
		void lookAt(glm::vec3 location);

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
		virtual void setLook(glm::vec3 look);

		/**
		 * Sets the position of the camera.
		 */
		void setPosition(glm::vec3 position, bool updateTargetPosition);

		/**
		 * Sets the speed of the camera.
		 */
		void setSpeed(float speed);

		/**
		 * Sets the target position of the camera.
		 * The camera will smoothly move to this position over time.
		 */
		void setTargetPosition(glm::vec3 targetPosition);

		/**
		 * Sets the up vector of the camera's coordinates system.
		 */
		void setUp(glm::vec3 up);

		/**
		 * Updates the view matrix, projection matrix, view Frustum etc.
		 * If changes have been applied to the camera (e.g. updating position) this function has to be called before 
		 * retrieving members of the camera. Otherwise the retrieved data might be outdated.
		 */
		virtual void update();

	protected:

		/**
		 * Checks if a given 3D-vector is a valid vector. If not, a runtime error is thrown.
		 */
		static void assertValidVector(const glm::vec3&);

		/**
		 * Recalculates the camera's view frustum (in view space).
		 */
		virtual void calcFrustum() = 0;

		/**
		 * Recalculates the camera's projection matrix.
		 */
		virtual void calcProjection() = 0;

		/**
		 * Recalculates the view space matrix of the camera.
		 */
		void calcView();

		PULCoordinateSystem mCoordSystem;
		Frustum mFrustum;
		nex::Logger mLogger;
		glm::mat4 mProjection;
		glm::vec3 mTargetPosition;
		glm::vec3 mRight;
		glm::mat4 mView;
		float mFarDistance;
		float mNearDistance;
		float mCameraSpeed;
	};

	/**
	 * A base class for perspective cameras.
	 */
	class PerspectiveCamera : public Camera
	{
	public:
		explicit PerspectiveCamera(float aspectRatio = 16.0f / 9.0f,
			float fovY = 45.0f, // the vertical field of view (in degrees)
			float nearDistance = 0.1f, // the distance to the near clipping plane
			float farDistance = 100.0f, // the distance to the far clipping plane
			PULCoordinateSystem coordinateSystem = PULCoordinateSystem()
		);

		PerspectiveCamera(glm::vec3 position, glm::vec3 look, glm::vec3 up);

		/**
		 * Enables/Disables zooming 
		 */
		void enableZoom(bool enable);

		void frameUpdate(Input* input, float frameTime) override;


		/**
		 * Provides the aspect ratio of the camera's canvas.
		 */
		float getAspectRatio() const;

		/**
		 * Provides the vertical field of view angle of the camera.
		 */
		float getFovY() const;


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

	protected:

		void calcProjection() override;
		void calcFrustum() override;

		float mAspectRatio;
		float mFovY;
		bool mZoomEnabled;
	};

	class OrthographicCamera : public Camera
	{
	public:
		OrthographicCamera(
			float width,
			float height,
			float nearDistance = 0.1f,
			float farDistance = 100.0f,
			PULCoordinateSystem coordSystem = PULCoordinateSystem()
		);

		float getHeight() const;
		float getWidth();

		void setHeight(float height);
		void setWidth(float width);

		protected:

			void calcFrustum() override;
			void calcProjection() override;

			float mHalfHeight;
			float mHalfWidth;
	};
}