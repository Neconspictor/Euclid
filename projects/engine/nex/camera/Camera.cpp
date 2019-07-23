#include <nex/camera/Camera.hpp>
#include <nex/Input.hpp>

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>
#include "nex/util/ExceptionHandling.hpp"
#include "nex/math/Ray.hpp"
#include "nex/math/Constant.hpp"


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

	const Frustum& Camera::getFrustum() const
	{
		return mFrustum;
	}

	const Frustum& Camera::getFrustumWorld() const
	{
		return mFrustumWorld;
	}

	float Camera::getSpeed() const
	{
		return mCameraSpeed;
	}

	void Camera::calcFrustumWorld()
	{
		auto& f = mFrustumWorld;
		const auto& g = mFrustum;
		const auto inverseView = inverse(mView);

		f.corners[(unsigned)FrustumCorners::NearLeftBottom] = glm::vec3(inverseView * glm::vec4(g.corners[(unsigned)FrustumCorners::NearLeftBottom], 1.0f));
		f.corners[(unsigned)FrustumCorners::NearLeftTop] = glm::vec3(inverseView * glm::vec4(g.corners[(unsigned)FrustumCorners::NearLeftTop], 1.0f));
		f.corners[(unsigned)FrustumCorners::NearRightBottom] = glm::vec3(inverseView * glm::vec4(g.corners[(unsigned)FrustumCorners::NearRightBottom], 1.0f));
		f.corners[(unsigned)FrustumCorners::NearRightTop] = glm::vec3(inverseView * glm::vec4(g.corners[(unsigned)FrustumCorners::NearRightTop], 1.0f));

		f.corners[(unsigned)FrustumCorners::FarLeftBottom] = glm::vec3(inverseView * glm::vec4(g.corners[(unsigned)FrustumCorners::FarLeftBottom], 1.0f));
		f.corners[(unsigned)FrustumCorners::FarLeftTop] = glm::vec3(inverseView * glm::vec4(g.corners[(unsigned)FrustumCorners::FarLeftTop], 1.0f));
		f.corners[(unsigned)FrustumCorners::FarRightBottom] = glm::vec3(inverseView * glm::vec4(g.corners[(unsigned)FrustumCorners::FarRightBottom], 1.0f));
		f.corners[(unsigned)FrustumCorners::FarRightTop] = glm::vec3(inverseView * glm::vec4(g.corners[(unsigned)FrustumCorners::FarRightTop], 1.0f));

		// For transforming the planes, we use the fact, that 
		// planes can be interpreted as 4D vectors. The transformed plane p' of p 
		// is than given by:
		// p' = transpose(inverse(trafo)) * p
		const auto transposeInverse = transpose(mView);

		f.planes[(unsigned)FrustumPlane::Near] = transformWithTransposeInverse(transposeInverse, g.planes[(unsigned)FrustumPlane::Near]);
		f.planes[(unsigned)FrustumPlane::Far] = transformWithTransposeInverse(transposeInverse, g.planes[(unsigned)FrustumPlane::Far]);
		f.planes[(unsigned)FrustumPlane::Left] = transformWithTransposeInverse(transposeInverse, g.planes[(unsigned)FrustumPlane::Left]);
		f.planes[(unsigned)FrustumPlane::Right] = transformWithTransposeInverse(transposeInverse, g.planes[(unsigned)FrustumPlane::Right]);
		f.planes[(unsigned)FrustumPlane::Bottom] = transformWithTransposeInverse(transposeInverse, g.planes[(unsigned)FrustumPlane::Bottom]);
		f.planes[(unsigned)FrustumPlane::Top] = transformWithTransposeInverse(transposeInverse, g.planes[(unsigned)FrustumPlane::Top]);
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

	const glm::mat4& Camera::getPrevView() const
	{
		return mPrevView;
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

	void Camera::update(bool inverse)
	{
		if (!inverse)
			mRight = normalize(cross(mCoordSystem.look, mCoordSystem.up));
		else
			mRight = normalize(cross(mCoordSystem.up, mCoordSystem.look));
		calcView();
		calcProjection();
		calcFrustum();
		calcFrustumWorld();
	}

	void Camera::setView(const glm::mat4 & view, bool resetPrev)
	{
		mPrevView = mView;
		mView = view;
		if (resetPrev) {
			mPrevView = view;
		}
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
		mPrevView = mView;
		mView = glm::lookAt(
			mCoordSystem.position,
			mCoordSystem.position + mCoordSystem.look,
			mCoordSystem.up
		);
	}

	PerspectiveCamera::PerspectiveCamera(unsigned width, unsigned height, float fovY, float nearDistance, float farDistance,
		PULCoordinateSystem coordinateSystem) : Camera(nearDistance, farDistance, std::move(coordinateSystem)), mFovY(fovY),
		mWidth(width), mHeight(height), mZoomEnabled(true)
	{
		if (height == 0) throw_with_trace(std::invalid_argument("PerspectiveCamera: height parameter mustn't be 0!"));
		mAspectRatio = width / (float)height;
	}

	nex::Ray PerspectiveCamera::calcScreenRay(const glm::ivec2& screenPosition) const
	{
		const auto look = normalize(getLook());
		const auto right = normalize(glm::cross(look, normalize(getUp())));
		const auto up = normalize(glm::cross(right, look));
		const auto nearD = getNearDistance();
		const auto tanFovYHalfth = tanf(getFovY() / 2.0f);
		const auto aspectRatio = getAspectRatio();

		// normalize screen position to [-1, 1] x [-1, 1]
		// Note: If screen position is out of range, the normalized position won't be in the target range,
		// but the followed calculations will be correct nevertheless.
		const float height = (static_cast<int>(mHeight) - screenPosition.y) / (float)mHeight;

		glm::vec2 normalizedPosition = 2.0f * glm::vec2(screenPosition.x / (float)mWidth, height) - 1.0f;
		
		// Compute direction vectors of the ray for each axis in the camera coordinate system in world space
		const glm::vec3 lookComponent = look * nearD;
		const glm::vec3 rightComponent = normalizedPosition.x * right * aspectRatio * tanFovYHalfth * nearD;
		const glm::vec3 upComponent = normalizedPosition.y * up * tanFovYHalfth * nearD;

		// the linear combination of the component vectors give as the ray direction to the screen position
		const glm::vec3 dir = normalize(lookComponent + rightComponent + upComponent);
		const glm::vec3 origin = getPosition() + dir * nearD;

		return nex::Ray(origin, dir);;
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

			setFovY(mFovY - glm::radians((float)yOffset));
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

	void PerspectiveCamera::setDimension(unsigned width, unsigned height)
	{
		if (height == 0) throw_with_trace(std::invalid_argument("PerspectiveCamera::setDimension: height parameter mustn't be 0!"));
		mWidth = width;
		mHeight = height;
		mAspectRatio = width / (float) height;
	}

	void PerspectiveCamera::setFovY(float fovY)
	{
		static const auto minVal = glm::radians(10.0f);
		static const auto maxVal = 0.75f * PI;
		mFovY = std::clamp<float>(fovY, minVal, maxVal);
	}


	void PerspectiveCamera::calcFrustum()
	{
		const auto zNear = getViewSpaceZfromDistance(mNearDistance);
		const auto zFar = getViewSpaceZfromDistance(mFarDistance);


		const float halfFovY = mFovY/2.0;

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


		mFrustum.corners[(unsigned)FrustumCorners::NearLeftBottom] = glm::vec3(halfWidthLeft * zNear, halfHeightBottom * zNear, zNear);
		mFrustum.corners[(unsigned)FrustumCorners::NearLeftTop] = glm::vec3(halfWidthLeft * zNear, halfHeightTop * zNear, zNear);
		mFrustum.corners[(unsigned)FrustumCorners::NearRightBottom] = glm::vec3(halfWidthRight * zNear, halfHeightBottom * zNear, zNear);
		mFrustum.corners[(unsigned)FrustumCorners::NearRightTop] = glm::vec3(halfWidthRight * zNear, halfHeightTop * zNear, zNear);

		mFrustum.corners[(unsigned)FrustumCorners::FarLeftBottom] = glm::vec3(halfWidthLeft * zFar, halfHeightBottom * zFar, zFar);
		mFrustum.corners[(unsigned)FrustumCorners::FarLeftTop] = glm::vec3(halfWidthLeft * zFar, halfHeightTop * zFar, zFar);
		mFrustum.corners[(unsigned)FrustumCorners::FarRightBottom] = glm::vec3(halfWidthRight * zFar, halfHeightBottom * zFar, zFar);
		mFrustum.corners[(unsigned)FrustumCorners::FarRightTop] = glm::vec3(halfWidthRight * zFar, halfHeightTop * zFar, zFar);


		/**
		 * We define now the 6 planes of the frustum.
		 * For math derivation see 
		 * 'Mathematics for 3D Game Programming and Computer Graphics (Third Edition)' by Eric Lengyel, page 106 (Chapter 5.3.2 Frustum Planes)
		 * Note that we use the vertical field of view for calculating the focal length e, so the plane equations are slightly different.
		 * The equations have been also generalized, so that they are valid for left and right handed coordination systems.
		 */

		const float e = 1.0f / tan(halfFovY); // focal length (using vertical field of view)
		const float a = mAspectRatio; // aspect ratio width over height (note, Lengyel uses height over width)
		const float zA = getViewSpaceZfromDistance(a); // a as signed depth (for switching between left and right handed)
		const float zOne = getViewSpaceZfromDistance(1.0f); // One as signed depth (for switching between left and right handed)
		const float divLeftRight = std::sqrtf(e*e + a*a); // divisor for normalization
		const float divBottTop = std::sqrtf(e*e + 1.0f); // divisor for normalization

		mFrustum.planes[(unsigned)FrustumPlane::Near] = {0, 0, zOne, -mNearDistance };
		mFrustum.planes[(unsigned)FrustumPlane::Far] = { 0, 0, -zOne, mFarDistance };

		mFrustum.planes[(unsigned)FrustumPlane::Left] = { e / divLeftRight, 0, zA / divLeftRight, 0 };
		mFrustum.planes[(unsigned)FrustumPlane::Right] = { -e / divLeftRight, 0, zA / divLeftRight, 0 };
		mFrustum.planes[(unsigned)FrustumPlane::Bottom] = { 0, e / divBottTop, zOne / divBottTop, 0 };
		mFrustum.planes[(unsigned)FrustumPlane::Top] = { 0, -e / divBottTop, zOne / divBottTop, 0 };
	}

	void PerspectiveCamera::calcProjection()
	{
		mProjection = glm::perspective(mFovY, mAspectRatio, mNearDistance, mFarDistance);
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

	float OrthographicCamera::getWidth() const
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

		mFrustum.corners[(unsigned)FrustumCorners::NearLeftBottom] = glm::vec3(-mHalfWidth, -mHalfHeight, zNear);
		mFrustum.corners[(unsigned)FrustumCorners::NearLeftTop] = glm::vec3(-mHalfWidth, mHalfHeight, zNear);
		mFrustum.corners[(unsigned)FrustumCorners::NearRightBottom] = glm::vec3(mHalfWidth, -mHalfHeight, zNear);
		mFrustum.corners[(unsigned)FrustumCorners::NearRightTop] = glm::vec3(mHalfWidth, mHalfHeight, zNear);

		mFrustum.corners[(unsigned)FrustumCorners::FarLeftBottom] = glm::vec3(-mHalfWidth, -mHalfHeight, zFar);
		mFrustum.corners[(unsigned)FrustumCorners::FarLeftTop] = glm::vec3(-mHalfWidth, mHalfHeight, zFar);
		mFrustum.corners[(unsigned)FrustumCorners::FarRightBottom] = glm::vec3(mHalfWidth, -mHalfHeight, zFar);
		mFrustum.corners[(unsigned)FrustumCorners::FarRightTop] = glm::vec3(mHalfWidth, mHalfHeight, zFar);


		const float zOne = getViewSpaceZfromDistance(1.0f); // One as signed depth (for switching between left and right handed)

		mFrustum.planes[(unsigned)FrustumPlane::Near] = { 0, 0, zOne, -mNearDistance };
		mFrustum.planes[(unsigned)FrustumPlane::Far] = { 0, 0, -zOne, mFarDistance };

		mFrustum.planes[(unsigned)FrustumPlane::Left] =   { 1.0f,  0,     0, mHalfWidth  };
		mFrustum.planes[(unsigned)FrustumPlane::Right] =  { -1.0f, 0,     0, mHalfWidth  };
		mFrustum.planes[(unsigned)FrustumPlane::Bottom] = { 0,     1.0f,  0, mHalfHeight };
		mFrustum.planes[(unsigned)FrustumPlane::Top] =    { 0,     -1.0f, 0, mHalfHeight };
	}

	void OrthographicCamera::calcProjection()
	{
		mProjection = glm::ortho(-mHalfWidth, mHalfWidth, -mHalfHeight, mHalfHeight);
	}
}
