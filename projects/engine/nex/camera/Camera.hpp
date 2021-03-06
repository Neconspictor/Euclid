#pragma once

#include <glm/glm.hpp>
#include <nex/common/Log.hpp>
#include <nex/math/Plane.hpp>
#include "nex/math/Constant.hpp"

namespace nex
{
	class Input;
	class Ray;
	struct AABB;

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

		AABB calcAABB()const;
	};

	Frustum operator*(const Frustum& frustum, const glm::mat4& mat);
	Frustum operator*(const glm::mat4& mat, const Frustum& frustum);

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
		Camera();

	protected:
		Camera(
			float width,
			float height,
			float nearDistance = 0.1f, // the distance to the near clipping plane
			float farDistance = 100.0f, // the distance to the far clipping plane
			PULCoordinateSystem coordinateSystem = PULCoordinateSystem());

		Camera(
			float width,
			float height,
			glm::vec3 position, glm::vec3 look, glm::vec3 up);

	public:
		Camera(const Camera&) = default;
		Camera(Camera&&) = default;
		Camera& operator=(const Camera&) = default;
		Camera& operator=(Camera&&) = default;
		virtual ~Camera() = default;

		/**
		 * Calculates a frustum cluster element for a frustum of the camera.
		 *
		 * @param xOffset				: Relative offset for x-axis; Range: [0,1]
		 * @param yOffset				: Relative offset for y-axis; Range: [0,1]
		 * @param zNearOffset			: Relative offset (near) for z-Axis; Range: [0,1]
		 * @param zRange				: Relative range for z-axis; Range: [0,1];
		 * @param xClusterElementSize	: Relative width of the cluster element; Range: [0,1]
		 * @param yClusterElementSize	: Relative height of the cluster element; Range: [0,1]
		 */
		virtual Frustum calcClusterElementViewSpace(float xOffset, float yOffset,
			float zNearOffset, float zRange,
			float xClusterElementSize, float yClusterElementSize) const = 0;

		/**
		 * Calculates frustum corners in viewspace.
		 */
		virtual void calcFrustumCornersVS(glm::vec3(&frustumCorners)[8], float zNearDistance, float zFarDistance) const = 0;

		/**
		 * Calculates frustum corners in worldspace.
		 */
		void calcFrustumCornersWS(glm::vec3(&frustumCorners)[8], float zNearDistance, float zFarDistance) const;

		/**
		 * Applies changes for the current frame.
		 */
		virtual void frameUpdate(Input* input, float frameTime);

		/**
		 * Provides clipping info of the camera.
		 * This information can be useful e.g. for retrieving view space z from depth.
		 * x-component: farPlane * nearPlane
		 * y-component: - nearPlane + farPlane
		 * z-component: farPlane
		 * w-component: 1 if camera is perspective, otherwise 0.
		 */
		virtual glm::vec4 getClipInfo() const;

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
		 * Jitter matrix for the current frame.
		 */
		const glm::mat4& getJitter() const;

		/**
		 * Provides the current look direction of the camera.
		 */
		const glm::vec3& getLook() const;

		/**
		 * Provides the distance to the near clipping plane.
		 */
		float getNearDistance() const;

		/**
		 * Provides the near and far plane in viewspace
		 */
		glm::vec2 getNearFarPlaneViewSpace() const;

		/**
		 * Provides the position of the camera.
		 */
		const glm::vec3& getPosition() const;

		/**
		 * Provides the projection matrix of the camera.
		 * If a jitter matrix is active, the projection matrix will be jittered.
		 */
		const glm::mat4& getProjectionMatrix() const;

		/**
		 * Provides the right vector, which is defined by the cross product of look and up vector:
		 * right = cross(look, up)
		 */
		const glm::vec3& getRight() const;

		/**
		 * Provides the speed of the camera.
		 */
		float getSpeed() const;

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
		 * Provides the last calculated view matrix.
		 */
		const glm::mat4& getViewInv() const;

		/**
		 * Provides the previous view matrix.
		 */
		const glm::mat4& getViewPrev() const;

		/**
		 * Jittered view-projection matrix for the current frame.
		 */
		const glm::mat4& getViewProj() const;

		const glm::mat4& getViewProjInv() const;

		/**
		 * Jittered view-projection matrix for the previous frame.
		 */
		const glm::mat4& getViewProjPrev() const;

		float getHeight() const;
		float getWidth() const;

		/**
		 * Calculate viewspace z from a distance to camera
		 */
		static float getViewSpaceZfromDistance(float distance);

		/**
		 * Orients the camera so that it looks to a given location.
		 */
		void lookAt(glm::vec3 location);

		virtual void setDimension(float width, float height);

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
		 * Sets the jitter matrix for the current frame.
		 */
		void setJitter(const glm::mat4& mat);
		void setJitterVec(const glm::vec2& jitter);

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
		virtual void update(bool inverse = false);

		void setPrevView(const glm::mat4& prevView);

		void setProjection(const glm::mat4& projection);

		void setView(const glm::mat4& view, bool resetPrev);

		void setViewProj(const glm::mat4& viewProj);
		void setViewProjInv(const glm::mat4& viewProjInv);
		void setPrevViewProj(const glm::mat4& prevViewProj);

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

		/**
		 * Transforms frustum corners from viewspace to worldspace
		 */
		void transformToWS(glm::vec3(&frustumCornersWS)[8], const glm::vec3(&frustumCornersVS)[8]) const;

		float mCameraSpeed;
		PULCoordinateSystem mCoordSystem;
		float mDistanceFar;
		float mDistanceNear;
		Frustum mFrustum;
		Frustum mFrustumWorld;
		glm::mat4 mJitter;
		nex::Logger mLogger;
		glm::mat4 mProjection;
		glm::vec3 mRight;
		glm::vec3 mTargetPosition;
		glm::mat4 mView;
		glm::mat4 mViewPrev;
		glm::mat4 mViewProj;
		glm::mat4 mViewProjInv;
		glm::mat4 mViewProjPrev;
		glm::mat4 mViewInv;

		float mWidth;
		float mHeight;
		glm::vec2 mJitterVec;
		
	};

	/**
	 * A base class for perspective cameras.
	 */
	class PerspectiveCamera : public Camera
	{
	public:
		explicit PerspectiveCamera(float width, float height,
			float fovY = glm::radians(45.0f), // the vertical field of view (in radians)
			float nearDistance = 0.1f, // the distance to the near clipping plane
			float farDistance = 100.0f, // the distance to the far clipping plane
			PULCoordinateSystem coordinateSystem = PULCoordinateSystem()
		);

		Frustum calcClusterElementViewSpace(float xOffset, float yOffset,
			float zNearOffset, float zFarOffset,
			float xClusterElementSize, float yClusterElementSize) const override;

		void calcFrustumCornersVS(glm::vec3(&frustumCorners)[8], float zNearDistance, float zFarDistance) const override;

		nex::Ray calcScreenRay(const glm::ivec2& screenPosition, const glm::ivec2 screenDimension) const;

		/**
		 * Enables/Disables zooming 
		 */
		void enableZoom(bool enable);

		void frameUpdate(Input* input, float frameTime) override;

		glm::vec4 getClipInfo() const override;

		/**
		 * Provides the aspect ratio of the camera's canvas.
		 */
		float getAspectRatio() const;

		/**
		 * Provides the vertical field of view angle (measured in radians) of the camera.
		 */
		float getFovY() const;

		void setDimension(float width, float height) override;

		/**
		 * Sets the vertical field of view angle (measured in radians). 
		 * The provided angle will be clamped to the range [0, pi]
		 */
		void setFovY(float fovY);

	protected:

		void calcProjection() override;
		void calcFrustum() override;

		static glm::mat4 getPerspectiveProjection(float left, float right, float bottom, float top, float nearPlane, float farPlane);
		glm::mat4 __getProjectionMatrix(float texelOffsetX, float texelOffsetY);
		glm::vec4 getProjectionExtents(float texelOffsetX, float texelOffsetY);

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

		Frustum calcClusterElementViewSpace(float xOffset, float yOffset,
			float zNearOffset, float zRange,
			float xClusterElementSize, float yClusterElementSize) const override;

		void setDimension(float width, float height) override;

		protected:

			void calcFrustum() override;
			void calcProjection() override;

			float mHalfHeight;
			float mHalfWidth;
	};
}
