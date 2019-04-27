#pragma once

#include <glm/glm.hpp>
#include <nex/common/Log.hpp>
#include <nex/util/Math.hpp>

namespace nex
{
	class Input;

	enum class FrustumPlane
	{
		Near = 0,
		Far = 1,
		Left = 2,
		Right = 3,
		Bottom = 4,
		Top = 5
	};

	enum class FrustumCorners
	{
		FarLeftBottom = 0,
		FarLeftTop = 1,
		FarRightBottom = 2,
		FarRightTop = 3,

		NearLeftBottom = 4,
		NearLeftTop = 5,
		NearRightBottom = 6,
		NearRightTop = 7
	};

	struct Frustum
	{
		/**
		 * corner order: 
		 * 0: FarLeftBottom
		 * 1: FarLeftTop
		 * 2: FarRightBottom
		 * 3: FarRightTop
		 * 4: NearLeftBottom
		 * 5: NearLeftTop
		 * 6: NearRightBottom
		 * 7: NearRightTop
		 */
		glm::vec3 corners[8];

		/**
		 * plane order:
		 * 0: near 
		 * 1: far
		 * 2: left
		 * 3: right
		 * 4: bottom
		 * 5: top
		 */
		Plane planes[6];
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
		const Frustum& getFrustum() const;

		/**
		 * Provides the current calculated view frustum (in world space).
		 */
		const Frustum& getFrustumWorld() const;

		/**
		 * Provides the speed of the camera.
		 */
		float getSpeed() const;

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
		 * Provides the previous view matrix.
		 */
		const glm::mat4& getPrevView() const;

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
		 * Calculates the view frustum in world space.
		 */
		void calcFrustumWorld();

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
		Frustum mFrustumWorld;
		nex::Logger mLogger;
		glm::mat4 mProjection;
		glm::vec3 mTargetPosition;
		glm::vec3 mRight;
		glm::mat4 mView;
		glm::mat4 mPrevView;
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
