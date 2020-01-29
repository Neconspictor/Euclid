#include <nex/camera/Camera.hpp>
#include <nex/platform/Input.hpp>

#define GLM_ENABLE_EXPERIMENTAL 1
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>
#include "nex/util/ExceptionHandling.hpp"
#include "nex/math/Ray.hpp"
#include "nex/math/Constant.hpp"
#include <nex/math/BoundingBox.hpp>
#include <nex/math/Math.hpp>


namespace nex
{
	Camera::Camera()
	{
	}

	Camera::Camera(float width,
		float height, float nearDistance, float farDistance, PULCoordinateSystem coordinateSystem) :
		mCoordSystem(std::move(coordinateSystem)), mLogger("Camera"), mTargetPosition(mCoordSystem.position),
		mDistanceFar(farDistance), mDistanceNear(nearDistance), mCameraSpeed(5.0f),
		mJitter(glm::mat4(1.0f)),
		mWidth(width),
		mHeight(height),
		mJitterVec(0,0)
	{
		mTargetPosition = mCoordSystem.position;
	}

	Camera::Camera(float width,
		float height, glm::vec3 position, glm::vec3 look, glm::vec3 up) : Camera(width, height)
	{
		mCoordSystem.position = std::move(position);
		mTargetPosition = mCoordSystem.position;
		mCoordSystem.look = std::move(look);
		mCoordSystem.up = std::move(up);
	}

	void Camera::calcFrustumCornersWS(glm::vec3(&frustumCorners)[8], float zNearDistance, float zFarDistance) const
	{
		calcFrustumCornersVS(frustumCorners, zNearDistance, zFarDistance);
		transformToWS(frustumCorners, frustumCorners);
	}

	void Camera::frameUpdate(Input* input, float frameTime)
	{
		//smooth transition from current position to target position using lerping 
		// A damping factor slows down speed over time.
		static const float DAMPING = 1.0f;
		const float alpha = std::clamp<float>(frameTime*2.0*DAMPING, 0.0f, 1.0f);
		mCoordSystem.position = glm::lerp<glm::vec3>(mCoordSystem.position, mTargetPosition, glm::vec3(alpha));
	}

	glm::vec4 Camera::getClipInfo() const
	{
		return glm::vec4(mDistanceFar * mDistanceNear, -mDistanceNear + mDistanceFar, mDistanceFar, 0);
	}

	float Camera::getFarDistance() const
	{
		return mDistanceFar;
	}

	const Frustum& Camera::getFrustum() const
	{
		return mFrustum;
	}

	const Frustum& Camera::getFrustumWorld() const
	{
		return mFrustumWorld;
	}

	const glm::mat4& Camera::getJitter() const
	{
		return mJitter;
	}

	const glm::mat4& Camera::getViewProj() const
	{
		return mViewProj;
	}

	const glm::mat4& Camera::getViewProjPrev() const
	{
		return mViewProjPrev;
	}

	float Camera::getHeight() const
	{
		return mHeight;
	}

	float Camera::getWidth() const
	{
		return mWidth;
	}

	float Camera::getSpeed() const
	{
		return mCameraSpeed;
	}

	void Camera::calcFrustumWorld()
	{
		auto& f = mFrustumWorld;
		const auto& g = mFrustum;

		transformToWS(f.corners, g.corners);

		// For transforming the planes, we use the fact, that 
		// planes can be interpreted as 4D vectors. The transformed plane p' of p 
		// is than given by:
		// p' = transpose(inverse(trafo)) * p
		// Note: we need the transpose inverse of the inverse view matrix.
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
		return mDistanceNear;
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

	const glm::mat4& Camera::getViewInv() const
	{
		return mViewInv;
	}

	const glm::mat4& Camera::getViewPrev() const
	{
		return mViewPrev;
	}

	glm::vec2 Camera::getNearFarPlaneViewSpace() const
	{
		return {getViewSpaceZfromDistance(mDistanceNear), getViewSpaceZfromDistance(mDistanceFar)};
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

	void Camera::setDimension(float width, float height)
	{
		mWidth = width;
		mHeight = height;
	}

	void Camera::setNearDistance(float nearDistance)
	{
		mDistanceNear = nearDistance;
	}

	void Camera::setFarDistance(float farDistance)
	{
		mDistanceFar = farDistance;
	}

	void Camera::setLook(glm::vec3 look)
	{		
		assertValidVector(look);

		mCoordSystem.look = std::move(look);
	}

	void Camera::setJitter(const glm::mat4& mat)
	{
		mJitter = mat;		
	}

	void Camera::setJitterVec(const glm::vec2& jitter)
	{
		mJitterVec = jitter;
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
		assertValidVector(up);

		mCoordSystem.up = std::move(up);
	}

	void Camera::update(bool inverse)
	{
		mCoordSystem.look = normalize(mCoordSystem.look);
		mCoordSystem.up = normalize(mCoordSystem.up);

		if (!inverse)
			mRight = normalize(cross(mCoordSystem.look, mCoordSystem.up));
		else
			mRight = normalize(cross(mCoordSystem.up, mCoordSystem.look));
		calcView();
		calcProjection();

		mViewProjPrev = mViewProj;
		mViewProj = mProjection * mView;
		

		calcFrustum();
		calcFrustumWorld();
	}

	void Camera::setPrevView(const glm::mat4& prevView)
	{
		mViewPrev = prevView;
	}

	void Camera::setProjection(const glm::mat4& projection)
	{
		mProjection = projection;
	}

	void Camera::setView(const glm::mat4 & view, bool resetPrev)
	{
		mViewPrev = mView;
		mView = view;
		if (resetPrev) {
			mViewPrev = view;
		}
	}

	void Camera::setViewProj(const glm::mat4& viewProj)
	{
		mViewProj = viewProj;
	}

	void Camera::setPrevViewProj(const glm::mat4& prevViewProj)
	{
		mViewProjPrev = prevViewProj;
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
		mViewPrev = mView;
		mView = glm::lookAt(
			mCoordSystem.position,
			mCoordSystem.position + mCoordSystem.look,
			mCoordSystem.up
		);

		mViewInv = inverse(mView);
	}

	void Camera::transformToWS(glm::vec3(&frustumCornersWS)[8], const glm::vec3(&frustumCornersVS)[8]) const
	{
		frustumCornersWS[(unsigned)FrustumCorners::NearLeftBottom] = glm::vec3(mViewInv * glm::vec4(frustumCornersVS[(unsigned)FrustumCorners::NearLeftBottom], 1.0f));
		frustumCornersWS[(unsigned)FrustumCorners::NearLeftTop] = glm::vec3(mViewInv * glm::vec4(frustumCornersVS[(unsigned)FrustumCorners::NearLeftTop], 1.0f));
		frustumCornersWS[(unsigned)FrustumCorners::NearRightBottom] = glm::vec3(mViewInv * glm::vec4(frustumCornersVS[(unsigned)FrustumCorners::NearRightBottom], 1.0f));
		frustumCornersWS[(unsigned)FrustumCorners::NearRightTop] = glm::vec3(mViewInv * glm::vec4(frustumCornersVS[(unsigned)FrustumCorners::NearRightTop], 1.0f));

		frustumCornersWS[(unsigned)FrustumCorners::FarLeftBottom] = glm::vec3(mViewInv * glm::vec4(frustumCornersVS[(unsigned)FrustumCorners::FarLeftBottom], 1.0f));
		frustumCornersWS[(unsigned)FrustumCorners::FarLeftTop] = glm::vec3(mViewInv * glm::vec4(frustumCornersVS[(unsigned)FrustumCorners::FarLeftTop], 1.0f));
		frustumCornersWS[(unsigned)FrustumCorners::FarRightBottom] = glm::vec3(mViewInv * glm::vec4(frustumCornersVS[(unsigned)FrustumCorners::FarRightBottom], 1.0f));
		frustumCornersWS[(unsigned)FrustumCorners::FarRightTop] = glm::vec3(mViewInv * glm::vec4(frustumCornersVS[(unsigned)FrustumCorners::FarRightTop], 1.0f));
	}

	PerspectiveCamera::PerspectiveCamera(float width, float height, float fovY, float nearDistance, float farDistance,
		PULCoordinateSystem coordinateSystem) : Camera(width, height, nearDistance, farDistance, std::move(coordinateSystem)),
		mAspectRatio(width / height),
		mFovY(fovY),
		mZoomEnabled(true)
	{
	}

	nex::Ray PerspectiveCamera::calcScreenRay(const glm::ivec2& screenPosition, const glm::ivec2 screenDimension) const
	{
		const auto look = normalize(getLook());
		const auto right = normalize(glm::cross(look, normalize(getUp())));
		const auto up = normalize(glm::cross(right, look));
		const auto nearD = getNearDistance();
		const auto tanFovYHalfth = tanf(getFovY() / 2.0f);
		const auto aspectRatio = getAspectRatio();

		// normalize screen position to [-1, 1] x [-1, 1] (clip space)
		// Note: screen space has origin at top left corner, while cip space has origin at bottom left corner.
		// Therefore we have to flip the y axis.
		// Note: If screen position is out of range, the normalized position won't be in the target range,
		// but the followed calculations will be correct nevertheless.
		const float height = (screenDimension.y - screenPosition.y) / (float)screenDimension.y;
		glm::vec2 positionClipSpace = 2.0f * glm::vec2(screenPosition.x / (float)screenDimension.x, height) - 1.0f;
		
		// Compute direction vectors of the ray for each axis in the camera coordinate system
		const glm::vec3 lookComponent = look * nearD;
		const glm::vec3 rightComponent = positionClipSpace.x * right * aspectRatio * tanFovYHalfth * nearD;
		const glm::vec3 upComponent = positionClipSpace.y * up * tanFovYHalfth * nearD;

		// the linear combination of the component vectors give us the ray direction to the screen position
		const glm::vec3 dir = normalize(lookComponent + rightComponent + upComponent);
		const glm::vec3 origin = getPosition() + dir * nearD;

		return nex::Ray(origin, dir);
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

	glm::vec4 PerspectiveCamera::getClipInfo() const
	{
		glm::vec4 info = Camera::getClipInfo();
		info.w = 1;
		return info;
	}

	float PerspectiveCamera::getAspectRatio() const
	{
		return mAspectRatio;
	}

	float PerspectiveCamera::getFovY() const
	{
		return mFovY;
	}

	void PerspectiveCamera::setDimension(float width, float height)
	{
		Camera::setDimension(width, height);
		mAspectRatio = width / height;
	}

	void PerspectiveCamera::setFovY(float fovY)
	{
		static const auto minVal = glm::radians(10.0f);
		static const auto maxVal = 0.75f * PI;
		mFovY = std::clamp<float>(fovY, minVal, maxVal);
	}


	void PerspectiveCamera::calcFrustum()
	{
		const auto zNear = getViewSpaceZfromDistance(mDistanceNear);
		const auto zFar = getViewSpaceZfromDistance(mDistanceFar);
		const float halfFovY = mFovY/2.0f;

		calcFrustumCornersVS(mFrustum.corners, mDistanceNear, mDistanceFar);

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

		mFrustum.planes[(unsigned)FrustumPlane::Near] = {0, 0, zOne, zNear }; //-mNearDistance
		mFrustum.planes[(unsigned)FrustumPlane::Far] = { 0, 0, -zOne, -zFar }; //mFarDistance

		mFrustum.planes[(unsigned)FrustumPlane::Left] = { 0, e / divBottTop, zOne / divBottTop, 0 };
		mFrustum.planes[(unsigned)FrustumPlane::Right] = { 0, -e / divBottTop, zOne / divBottTop, 0 };

		mFrustum.planes[(unsigned)FrustumPlane::Bottom] = { e / divLeftRight, 0, zA / divLeftRight, 0 };
		mFrustum.planes[(unsigned)FrustumPlane::Top] = { -e / divLeftRight, 0, zA / divLeftRight, 0 };
	}
	
	Frustum PerspectiveCamera::calcClusterElementViewSpace(
		float xOffset, float yOffset,
		float zNearOffset, float zRange,
		float xClusterElementSize, float yClusterElementSize) const {

		Frustum elem;

		const auto clipRange = getFarDistance() - getNearDistance();
		const auto zNearDistance = getNearDistance() + clipRange * zNearOffset;
		const auto zFarDistance = zNearDistance + clipRange * zRange;


		const auto zNear = getViewSpaceZfromDistance(zNearDistance);
		const auto zFar = getViewSpaceZfromDistance(zFarDistance);

		const auto halfFovY = getFovY() / 2.0f;
		const auto globalFrustumTop = tan(halfFovY);
		const auto globalFrustumBottom = -globalFrustumTop;
		const auto globalFrustumRight = getAspectRatio() * tan(halfFovY);
		const auto globalFrustumLeft = -globalFrustumRight;

		const auto globalFrustumWidth = globalFrustumRight - globalFrustumLeft;
		const auto globalFrustumHeight = globalFrustumTop - globalFrustumBottom;
		const auto tileWidth = globalFrustumWidth * xClusterElementSize;
		const auto tileHeight = globalFrustumHeight * yClusterElementSize;

		const auto widthOffset = globalFrustumWidth * xOffset;
		const auto widthOffsetViewSpaceZ = getViewSpaceZfromDistance(widthOffset);
		const auto heightOffset = globalFrustumHeight * yOffset;
		const auto heightOffsetViewSpaceZ = getViewSpaceZfromDistance(heightOffset);

		const auto left = globalFrustumLeft + widthOffset;
		const auto right = left + tileWidth;
		const auto bottom = globalFrustumBottom + heightOffset;
		const auto top = bottom + tileHeight;


		elem.corners[(unsigned)nex::FrustumCorners::NearLeftBottom] =
			glm::vec3(left * zNearDistance, bottom * zNearDistance, zNear);
		elem.corners[(unsigned)nex::FrustumCorners::NearLeftTop] =
			glm::vec3(left * zNearDistance, top * zNearDistance, zNear);
		elem.corners[(unsigned)nex::FrustumCorners::NearRightBottom] =
			glm::vec3(right * zNearDistance, bottom * zNearDistance, zNear);
		elem.corners[(unsigned)nex::FrustumCorners::NearRightTop] =
			glm::vec3(right * zNearDistance, top * zNearDistance, zNear);

		elem.corners[(unsigned)nex::FrustumCorners::FarLeftBottom] =
			glm::vec3(left * zFarDistance, bottom * zFarDistance, zFar);
		elem.corners[(unsigned)nex::FrustumCorners::FarLeftTop] =
			glm::vec3(left * zFarDistance, top * zFarDistance, zFar);
		elem.corners[(unsigned)nex::FrustumCorners::FarRightBottom] =
			glm::vec3(right * zFarDistance, bottom * zFarDistance, zFar);
		elem.corners[(unsigned)nex::FrustumCorners::FarRightTop] =
			glm::vec3(right * zFarDistance, top * zFarDistance, zFar);


		/**
		 * We define now the 6 planes of the frustum.
		 * For math derivation see
		 * 'Mathematics for 3D Game Programming and Computer Graphics (Third Edition)' by Eric Lengyel, page 106 (Chapter 5.3.2 Frustum Planes)
		 * Note that we use the vertical field of view for calculating the focal length e, so the plane equations are slightly different.
		 * The equations have been also generalized, so that they are valid for left and right handed coordination systems.
		 * Additionally the distance parameter for (left,right,bottom,top) was added for supporting xOffset and yOffset parameter.
		 */

		const float e = 1.0f / tan(halfFovY); // focal length (using vertical field of view)
		const float a = mAspectRatio; // aspect ratio width over height (note, Lengyel uses height over width)
		const float zA = getViewSpaceZfromDistance(a); // a as signed depth (for switching between left and right handed)
		const float zOne = getViewSpaceZfromDistance(1.0f); // One as signed depth (for switching between left and right handed)
		const float divLeftRight = std::sqrtf(e * e + a * a); // divisor for normalization
		const float divBottTop = std::sqrtf(e * e + 1.0f); // divisor for normalization

		elem.planes[(unsigned)FrustumPlane::Near] = { 0, 0, zOne, zNear }; //-mNearDistance
		elem.planes[(unsigned)FrustumPlane::Far] = { 0, 0, -zOne, -zFar }; //mFarDistance

		elem.planes[(unsigned)FrustumPlane::Left] = { 0, e / divBottTop, zOne / divBottTop, widthOffsetViewSpaceZ };
		elem.planes[(unsigned)FrustumPlane::Right] = { 0, -e / divBottTop, zOne / divBottTop, widthOffsetViewSpaceZ };

		elem.planes[(unsigned)FrustumPlane::Bottom] = { e / divLeftRight, 0, zA / divLeftRight, heightOffsetViewSpaceZ };
		elem.planes[(unsigned)FrustumPlane::Top] = { -e / divLeftRight, 0, zA / divLeftRight, heightOffsetViewSpaceZ };

		return elem;
	}

	void PerspectiveCamera::calcFrustumCornersVS(glm::vec3(&frustumCorners)[8], float zNearDistance, float zFarDistance) const
	{
		const auto zNear = getViewSpaceZfromDistance(zNearDistance);
		const auto zFar = getViewSpaceZfromDistance(zFarDistance);


		const float halfFovY = mFovY / 2.0f;

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


		frustumCorners[(unsigned)FrustumCorners::NearLeftBottom] = glm::vec3(halfWidthLeft * zNearDistance, halfHeightBottom * zNearDistance, zNear);
		frustumCorners[(unsigned)FrustumCorners::NearLeftTop] = glm::vec3(halfWidthLeft * zNearDistance, halfHeightTop * zNearDistance, zNear);
		frustumCorners[(unsigned)FrustumCorners::NearRightBottom] = glm::vec3(halfWidthRight * zNearDistance, halfHeightBottom * zNearDistance, zNear);
		frustumCorners[(unsigned)FrustumCorners::NearRightTop] = glm::vec3(halfWidthRight * zNearDistance, halfHeightTop * zNearDistance, zNear);

		frustumCorners[(unsigned)FrustumCorners::FarLeftBottom] = glm::vec3(halfWidthLeft * zFarDistance, halfHeightBottom * zFarDistance, zFar);
		frustumCorners[(unsigned)FrustumCorners::FarLeftTop] = glm::vec3(halfWidthLeft * zFarDistance, halfHeightTop * zFarDistance, zFar);
		frustumCorners[(unsigned)FrustumCorners::FarRightBottom] = glm::vec3(halfWidthRight * zFarDistance, halfHeightBottom * zFarDistance, zFar);
		frustumCorners[(unsigned)FrustumCorners::FarRightTop] = glm::vec3(halfWidthRight * zFarDistance, halfHeightTop * zFarDistance, zFar);

	}

	void PerspectiveCamera::calcProjection()
	{
		mProjection = glm::perspective(mFovY, mAspectRatio, mDistanceNear, mDistanceFar);
		mProjection = mJitter * mProjection;
		//mProjection = __getProjectionMatrix(mJitterVec.x, mJitterVec.y);
		
	}

	glm::mat4 PerspectiveCamera::getPerspectiveProjection(float left, float right, float bottom, float top, float nearPlane, float farPlane)
	{
		float x = (2.0f * nearPlane) / (right - left);
		float y = (2.0f * nearPlane) / (top - bottom);
		float a = (right + left) / (right - left);
		float b = (top + bottom) / (top - bottom);
		float c = -(farPlane + nearPlane) / (farPlane - nearPlane);
		float d = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);
		float e = -1.0f;

		glm::mat4 m;
		m[0][0] = x; m[1][0] = 0; m[2][0] = a; m[3][0] = 0;
		m[0][1] = 0; m[1][1] = y; m[2][1] = b; m[3][1] = 0;
		m[0][2] = 0; m[1][2] = 0; m[2][2] = c; m[3][2] = d;
		m[0][3] = 0; m[1][3] = 0; m[2][3] = e; m[3][3] = 0;
		return m;
	}

	glm::vec4 PerspectiveCamera::getProjectionExtents(float texelOffsetX, float texelOffsetY)
	{
		
		float oneExtentY = std::tanf(0.5f * mFovY);
		float oneExtentX = oneExtentY * mAspectRatio;
		float texelSizeX = oneExtentX / (0.5f * mWidth);
		float texelSizeY = oneExtentY / (0.5f * mHeight);
		float oneJitterX = texelSizeX * texelOffsetX;
		float oneJitterY = texelSizeY * texelOffsetY;

		return glm::vec4(oneExtentX, oneExtentY, oneJitterX, oneJitterY);// xy = frustum extents at distance 1, zw = jitter at distance 1
	}

	glm::mat4 PerspectiveCamera::__getProjectionMatrix(float texelOffsetX, float texelOffsetY)
	{
		auto extents = getProjectionExtents(texelOffsetX, texelOffsetY);

		float cf = mDistanceFar;
		float cn = mDistanceNear;
		float xm = extents.z - extents.x;
		float xp = extents.z + extents.x;
		float ym = extents.w - extents.y;
		float yp = extents.w + extents.y;

		return getPerspectiveProjection(xm * cn, xp * cn, ym * cn, yp * cn, cn, cf);
	}

	OrthographicCamera::OrthographicCamera(float width, float height, float nearDistance, float farDistance,
		PULCoordinateSystem coordSystem) : Camera(width, height, nearDistance, farDistance, std::move(coordSystem)),
	mHalfHeight(height / 2.0f), mHalfWidth(width / 2.0f)
	{
		assert(mHalfHeight != 0.0f);
		assert(mHalfWidth != 0.0f);
	}

	Frustum OrthographicCamera::calcClusterElementViewSpace(float xOffset, float yOffset, float zNearOffset, float zRange, float xClusterElementSize, float yClusterElementSize) const
	{
		//TODO
		return Frustum();
	}

	void OrthographicCamera::setDimension(float width, float height)
	{
		Camera::setDimension(width, height);
		mHalfHeight = height / 2.0f;
		mHalfWidth = width / 2.0f;
	}

	void OrthographicCamera::calcFrustum()
	{
		const auto zNear = getViewSpaceZfromDistance(mDistanceNear);
		const auto zFar = getViewSpaceZfromDistance(mDistanceFar);

		mFrustum.corners[(unsigned)FrustumCorners::NearLeftBottom] = glm::vec3(-mHalfWidth, -mHalfHeight, zNear);
		mFrustum.corners[(unsigned)FrustumCorners::NearLeftTop] = glm::vec3(-mHalfWidth, mHalfHeight, zNear);
		mFrustum.corners[(unsigned)FrustumCorners::NearRightBottom] = glm::vec3(mHalfWidth, -mHalfHeight, zNear);
		mFrustum.corners[(unsigned)FrustumCorners::NearRightTop] = glm::vec3(mHalfWidth, mHalfHeight, zNear);

		mFrustum.corners[(unsigned)FrustumCorners::FarLeftBottom] = glm::vec3(-mHalfWidth, -mHalfHeight, zFar);
		mFrustum.corners[(unsigned)FrustumCorners::FarLeftTop] = glm::vec3(-mHalfWidth, mHalfHeight, zFar);
		mFrustum.corners[(unsigned)FrustumCorners::FarRightBottom] = glm::vec3(mHalfWidth, -mHalfHeight, zFar);
		mFrustum.corners[(unsigned)FrustumCorners::FarRightTop] = glm::vec3(mHalfWidth, mHalfHeight, zFar);


		const float zOne = getViewSpaceZfromDistance(1.0f); // One as signed depth (for switching between left and right handed)

		mFrustum.planes[(unsigned)FrustumPlane::Near] = { 0, 0, zOne, -mDistanceNear };
		mFrustum.planes[(unsigned)FrustumPlane::Far] = { 0, 0, -zOne, mDistanceFar };

		mFrustum.planes[(unsigned)FrustumPlane::Left] =   { 1.0f,  0,     0, mHalfWidth  };
		mFrustum.planes[(unsigned)FrustumPlane::Right] =  { -1.0f, 0,     0, mHalfWidth  };
		mFrustum.planes[(unsigned)FrustumPlane::Bottom] = { 0,     1.0f,  0, mHalfHeight };
		mFrustum.planes[(unsigned)FrustumPlane::Top] =    { 0,     -1.0f, 0, mHalfHeight };
	}

	void OrthographicCamera::calcProjection()
	{
		mProjection = glm::ortho(-mHalfWidth, mHalfWidth, -mHalfHeight, mHalfHeight, mDistanceNear, mDistanceFar);
		//mProjection = glm::ortho(-mHalfWidth, mHalfWidth, -mHalfHeight, mHalfHeight);
	}

	Frustum operator*(const Frustum& frustum, const glm::mat4& mat)
	{
		return mat * frustum;
	}
	Frustum operator*(const glm::mat4& mat, const Frustum& g)
	{
		Frustum f;

		f.corners[(unsigned)FrustumCorners::NearLeftBottom] = glm::vec3(mat * glm::vec4(g.corners[(unsigned)FrustumCorners::NearLeftBottom], 1.0f));
		f.corners[(unsigned)FrustumCorners::NearLeftTop] = glm::vec3(mat * glm::vec4(g.corners[(unsigned)FrustumCorners::NearLeftTop], 1.0f));
		f.corners[(unsigned)FrustumCorners::NearRightBottom] = glm::vec3(mat * glm::vec4(g.corners[(unsigned)FrustumCorners::NearRightBottom], 1.0f));
		f.corners[(unsigned)FrustumCorners::NearRightTop] = glm::vec3(mat * glm::vec4(g.corners[(unsigned)FrustumCorners::NearRightTop], 1.0f));

		f.corners[(unsigned)FrustumCorners::FarLeftBottom] = glm::vec3(mat * glm::vec4(g.corners[(unsigned)FrustumCorners::FarLeftBottom], 1.0f));
		f.corners[(unsigned)FrustumCorners::FarLeftTop] = glm::vec3(mat * glm::vec4(g.corners[(unsigned)FrustumCorners::FarLeftTop], 1.0f));
		f.corners[(unsigned)FrustumCorners::FarRightBottom] = glm::vec3(mat * glm::vec4(g.corners[(unsigned)FrustumCorners::FarRightBottom], 1.0f));
		f.corners[(unsigned)FrustumCorners::FarRightTop] = glm::vec3(mat * glm::vec4(g.corners[(unsigned)FrustumCorners::FarRightTop], 1.0f));

		// For transforming the planes, we use the fact, that 
		// planes can be interpreted as 4D vectors. The transformed plane p' of p 
		// is than given by:
		// p' = transpose(inverse(trafo)) * p
		const auto transposeInverse = transpose(inverse(mat));

		f.planes[(unsigned)FrustumPlane::Near] = transformWithTransposeInverse(transposeInverse, g.planes[(unsigned)FrustumPlane::Near]);
		f.planes[(unsigned)FrustumPlane::Far] = transformWithTransposeInverse(transposeInverse, g.planes[(unsigned)FrustumPlane::Far]);
		f.planes[(unsigned)FrustumPlane::Left] = transformWithTransposeInverse(transposeInverse, g.planes[(unsigned)FrustumPlane::Left]);
		f.planes[(unsigned)FrustumPlane::Right] = transformWithTransposeInverse(transposeInverse, g.planes[(unsigned)FrustumPlane::Right]);
		f.planes[(unsigned)FrustumPlane::Bottom] = transformWithTransposeInverse(transposeInverse, g.planes[(unsigned)FrustumPlane::Bottom]);
		f.planes[(unsigned)FrustumPlane::Top] = transformWithTransposeInverse(transposeInverse, g.planes[(unsigned)FrustumPlane::Top]);

		return f;
	}
	nex::AABB Frustum::calcAABB() const
	{
		AABB box;

		for (unsigned i = 0; i < 8; ++i) {
			box.min = minVec(box.min, corners[i]);
			box.max = maxVec(box.max, corners[i]);
		}

		return box;
	}
}