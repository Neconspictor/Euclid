#include <nex/camera/Camera.hpp>
#include <stdexcept>
#include <nex/util/Math.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/Input.hpp>

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>


namespace nex
{
	Camera::Camera(float nearDistance, float farDistance, PULCoordinateSystem coordinateSystem) :
		mCoordSystem(std::move(coordinateSystem)), mLogger("Camera"), mTargetPosition(mCoordSystem.position),
		mFarDistance(farDistance), mNearDistance(nearDistance), mCameraSpeed(5.0f)
	{
		mTargetPosition = mCoordSystem.position;
	}

	Camera::Camera(glm::vec3 position, glm::vec3 look, glm::vec3 up) : Camera()
	{
		mCoordSystem.position = std::move(position);
		mTargetPosition = mCoordSystem.position;
		mCoordSystem.look = std::move(look);
		mCoordSystem.up = std::move(up);
	}

	void Camera::frameUpdate(Input* input, float frameTime)
	{
		//smooth transition from current position to target position using lerping 
		// A damping factor slows down speed over time.
		static const float DAMPING = 1.0f;
		const float alpha = std::clamp<float>(frameTime*2.0*DAMPING, 0.0f, 1.0f);
		mCoordSystem.position = glm::lerp<glm::vec3>(mCoordSystem.position, mTargetPosition, glm::vec3(alpha));
	}

	float Camera::getFarDistance() const
	{
		return mFarDistance;
	}

	const Frustum& Camera::getFrustum()
	{
		return mFrustum;
	}

	float Camera::getSpeed() const
	{
		return mCameraSpeed;
	}

	Frustum Camera::calcFrustumWorld() const
	{
		Frustum f = mFrustum;
		const auto inverseView = inverse(mView);

		f.farLeftBottom = glm::vec3(inverseView * glm::vec4(f.farLeftBottom, 1.0f));
		f.farLeftTop = glm::vec3(inverseView * glm::vec4(f.farLeftTop, 1.0f));
		f.farRightBottom = glm::vec3(inverseView * glm::vec4(f.farRightBottom, 1.0f));
		f.farRightTop = glm::vec3(inverseView * glm::vec4(f.farRightTop, 1.0f));

		f.nearLeftBottom = glm::vec3(inverseView * glm::vec4(f.nearLeftBottom, 1.0f));
		f.nearLeftTop = glm::vec3(inverseView * glm::vec4(f.nearLeftTop, 1.0f));
		f.nearRightBottom = glm::vec3(inverseView * glm::vec4(f.nearRightBottom, 1.0f));
		f.nearRightTop = glm::vec3(inverseView * glm::vec4(f.nearRightTop, 1.0f));

		return f;
	}

	const glm::vec3& Camera::getLook() const
	{
		return mCoordSystem.look;
	}

	const glm::mat4& Camera::getProjectionMatrix() const
	{
		return mProjection;
	}

	const glm::vec3& Camera::getPosition() const
	{
		return mCoordSystem.position;
	}

	float Camera::getNearDistance() const
	{
		return mNearDistance;
	}

	const glm::vec3& Camera::getRight() const
	{
		return mRight;
	}

	const glm::vec3& Camera::getTargetPosition() const
	{
		return mTargetPosition;
	}

	const glm::vec3& Camera::getUp() const
	{
		return mCoordSystem.up;
	}

	const glm::mat4& Camera::getView() const
	{
		return mView;
	}

	glm::vec2 Camera::getNearFarPlaneViewSpace() const
	{
		return {getViewSpaceZfromDistance(mNearDistance), getViewSpaceZfromDistance(mFarDistance)};
	}

	float Camera::getViewSpaceZfromDistance(float distance)
	{
#ifndef USE_LEFT_HANDED_COORDINATE_SYSTEM
		distance *= -1; // the z-axis is inverted on right handed systems
#endif
		return distance;
	}

	void Camera::lookAt(glm::vec3 location)
	{
		mCoordSystem.look = normalize(location - mCoordSystem.position);
	}

	void Camera::setNearDistance(float nearDistance)
	{
		mNearDistance = nearDistance;
	}

	void Camera::setFarDistance(float farDistance)
	{
		mFarDistance = farDistance;
	}

	void Camera::setLook(glm::vec3 look)
	{
		look = normalize(look);
		assertValidVector(look);

		mCoordSystem.look = std::move(look);
	}

	void Camera::setPosition(glm::vec3 position, bool updateTargetPosition)
	{
		mCoordSystem.position = std::move(position);
		if (updateTargetPosition)
		{
			mTargetPosition = mCoordSystem.position;
		}
	}

	void Camera::setSpeed(float speed)
	{
		mCameraSpeed = speed;
	}

	void Camera::setTargetPosition(glm::vec3 targetPosition)
	{
		mTargetPosition = targetPosition;
	}

	void Camera::setUp(glm::vec3 up)
	{
		up = normalize(up);
		assertValidVector(up);

		mCoordSystem.up = std::move(up);
	}

	void Camera::update()
	{
		mRight = normalize(cross(mCoordSystem.look, mCoordSystem.up));
		calcProjection();
		calcFrustum();
		calcView();
	}

	void Camera::assertValidVector(const glm::vec3& vec)
	{
		if (isnan(vec.x) ||
			isnan(vec.y) ||
			isnan(vec.z))
		{
			throw_with_trace(std::runtime_error("Camera::assertValidVector: specified a non valid vector!"));
		}
	}

	void Camera::calcView()
	{
		mView = glm::lookAt(
			mCoordSystem.position,
			mCoordSystem.position + mCoordSystem.look,
			mCoordSystem.up
		);
	}

	PerspectiveCamera::PerspectiveCamera(float aspectRatio, float fovY, float nearDistance, float farDistance,
		PULCoordinateSystem coordinateSystem) : Camera(nearDistance, farDistance, std::move(coordinateSystem)), mAspectRatio(aspectRatio), mFovY(fovY),
		mZoomEnabled(true)
	{
	}

	PerspectiveCamera::PerspectiveCamera(glm::vec3 position, glm::vec3 look, glm::vec3 up) : PerspectiveCamera()
	{

		mCoordSystem.position = std::move(position);
		mTargetPosition = mCoordSystem.position;
		mCoordSystem.look = std::move(look);
		mCoordSystem.up = std::move(up);
	}

	void PerspectiveCamera::enableZoom(bool enable)
	{
		mZoomEnabled = enable;
	}

	void PerspectiveCamera::frameUpdate(Input* input, float frameTime)
	{
		if (mZoomEnabled)
		{
			double yOffset = input->getFrameScrollOffsetY();

			// zoom
			if (mFovY >= 1.0f && mFovY <= 45.0f)
				mFovY -= (float)yOffset;
			if (mFovY <= 1.0f)
				mFovY = 1.0f;
			if (mFovY >= 45.0f)
				mFovY = 45.0f;
		}

		Camera::frameUpdate(input, frameTime);
	}

	float PerspectiveCamera::getAspectRatio() const
	{
		return mAspectRatio;
	}

	float PerspectiveCamera::getFovY() const
	{
		return mFovY;
	}

	void PerspectiveCamera::setAspectRatio(float ratio)
	{
		mAspectRatio = ratio;
	}

	void PerspectiveCamera::setFovY(float fovY)
	{
		mFovY = fovY;
	}

	void PerspectiveCamera::calcFrustum()
	{
		const auto zNear = getViewSpaceZfromDistance(mNearDistance);
		const auto zFar = getViewSpaceZfromDistance(mFarDistance);


		const auto halfFovY = glm::radians(mFovY/2.0f);

		/**
		 * Calculate constants for the (half of the) width and height of the near and far planes. 
		 * 
		 * Math background:
		 * To calculate the corners of a view frustum plane, let z' = 'z coordinate in viewspace' and r = 'aspect ratio'.
		 * The corners than can be calculated in viewspace, by:
		 * 
		 * z = z'
		 * y = +/- tan(fovY/2) * z'  (positive for top corners, negative for bottom corners)
		 * x = +/- r * tan(fovY/2) * z' (positive for right corners, negative for left corners)
		 * 
		 **/
		const auto halfHeightTop = tan(halfFovY);
		const auto halfHeightBottom = -halfHeightTop;
		const auto halfWidthRight = mAspectRatio * tan(halfFovY);
		const auto halfWidthLeft = -halfWidthRight;


		mFrustum.nearLeftBottom = glm::vec3(halfWidthLeft * zNear, halfHeightBottom * zNear, zNear);
		mFrustum.nearLeftTop = glm::vec3(halfWidthLeft * zNear, halfHeightTop * zNear, zNear);
		mFrustum.nearRightBottom = glm::vec3(halfWidthRight * zNear, halfHeightBottom * zNear, zNear);
		mFrustum.nearRightTop = glm::vec3(halfWidthRight * zNear, halfHeightTop * zNear, zNear);

		mFrustum.farLeftBottom = glm::vec3(halfWidthLeft * zFar, halfHeightBottom * zFar, zFar);
		mFrustum.farLeftTop = glm::vec3(halfWidthLeft * zFar, halfHeightTop * zFar, zFar);
		mFrustum.farRightBottom = glm::vec3(halfWidthRight * zFar, halfHeightBottom * zFar, zFar);
		mFrustum.farRightTop = glm::vec3(halfWidthRight * zFar, halfHeightTop * zFar, zFar);
	}

	void PerspectiveCamera::calcProjection()
	{
		mProjection = glm::perspective(glm::radians(mFovY), mAspectRatio, mNearDistance, mFarDistance);
	}

	OrthographicCamera::OrthographicCamera(float width, float height, float nearDistance, float farDistance,
		PULCoordinateSystem coordSystem) : Camera(nearDistance, farDistance, std::move(coordSystem)),
	mHalfHeight(height / 2.0f), mHalfWidth(width / 2.0f)
	{
		assert(mHalfHeight != 0.0f);
		assert(mHalfWidth != 0.0f);
	}

	float OrthographicCamera::getHeight() const
	{
		return 2.0f * mHalfHeight;
	}

	float OrthographicCamera::getWidth()
	{
		return 2.0f * mHalfWidth;
	}

	void OrthographicCamera::setHeight(float height)
	{
		mHalfHeight = height / 2.0f;
	}

	void OrthographicCamera::setWidth(float width)
	{
		mHalfWidth = width / 2.0f;
	}

	void OrthographicCamera::calcFrustum()
	{
		const auto zNear = getViewSpaceZfromDistance(mNearDistance);
		const auto zFar = getViewSpaceZfromDistance(mFarDistance);

		mFrustum.nearLeftBottom = glm::vec3(-mHalfWidth, -mHalfHeight, zNear);
		mFrustum.nearLeftTop = glm::vec3(-mHalfWidth, mHalfHeight, zNear);
		mFrustum.nearRightBottom = glm::vec3(mHalfWidth, -mHalfHeight, zNear);
		mFrustum.nearRightTop = glm::vec3(mHalfWidth, mHalfHeight, zNear);

		mFrustum.farLeftBottom = glm::vec3(-mHalfWidth, -mHalfHeight, zFar);
		mFrustum.farLeftTop = glm::vec3(-mHalfWidth, mHalfHeight, zFar);
		mFrustum.farRightBottom = glm::vec3(mHalfWidth, -mHalfHeight, zFar);
		mFrustum.farRightTop = glm::vec3(mHalfWidth, mHalfHeight, zFar);
	}

	void OrthographicCamera::calcProjection()
	{
		mProjection = glm::ortho(-mHalfWidth, mHalfWidth, -mHalfHeight, mHalfHeight);
	}
}