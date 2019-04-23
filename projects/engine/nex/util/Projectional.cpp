#include <nex/util/Projectional.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <nex/util/Math.hpp>
#include <nex/util/ExceptionHandling.hpp>

using namespace std;
using namespace glm;

namespace nex
{
	PerspectiveCamera::PerspectiveCamera(float aspectRatio, float fovY, float nearDistance, float farDistance,
		PULCoordinateSystem coordinateSystem) : mAspectRatio(aspectRatio), mFovY(fovY),
		mCoordSystem(std::move(coordinateSystem)), mLogger("PerspectiveProjectional"), mTargetPosition(mCoordSystem.position),
		mRevalidate(true), mFarDistance(farDistance), mNearDistance(nearDistance)
	{
		setLook(mCoordSystem.look);
		setUp(mCoordSystem.up);
		update(true);
	}

	void PerspectiveCamera::calcView()
	{
		mView = glm::lookAt(
			mCoordSystem.position,
			mCoordSystem.position + mCoordSystem.look,
			mCoordSystem.up
		);
	}

	float PerspectiveCamera::getAspectRatio() const
	{
		return mAspectRatio;
	}

	float PerspectiveCamera::getFarDistance() const
	{
		return mFarDistance;
	}

	float PerspectiveCamera::getNearDistance() const
	{
		return mNearDistance;
	}

	const glm::vec3& PerspectiveCamera::getLook() const
	{
		return mCoordSystem.look;
	}

	float PerspectiveCamera::getFovY() const
	{
		return mFovY;
	}

	const Frustum& PerspectiveCamera::getFrustum()
	{
		return mFrustum;
	}

	Frustum PerspectiveCamera::calcFrustumWorld() const
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

	const glm::mat4& PerspectiveCamera::getProjectionMatrix() const
	{
		return mProjection;
	}

	const glm::vec3& PerspectiveCamera::getPosition() const
	{
		return mCoordSystem.position;
	}

	const glm::vec3& PerspectiveCamera::getRight() const
	{
		return mRight;
	}

	const glm::vec3& PerspectiveCamera::getUp() const
	{
		return mCoordSystem.up;
	}

	const glm::mat4& PerspectiveCamera::getView() const
	{
		return mView;
	}

	float PerspectiveCamera::getViewSpaceZfromDistance(float distance)
	{
#ifndef USE_LEFT_HANDED_COORDINATE_SYSTEM
		distance *= -1; // the z-axis is inverted on right handed systems
#endif
		return distance;
	}

	void PerspectiveCamera::lookAt(glm::vec3 location)
	{
		mCoordSystem.look = normalize(location - mCoordSystem.position);
		mRevalidate = true;
	}

	void PerspectiveCamera::setAspectRatio(float ratio)
	{
		mAspectRatio = ratio;
		mRevalidate = true;
	}

	void PerspectiveCamera::setFovY(float fovY)
	{
		mFovY = fovY;
		mRevalidate = true;
	}

	void PerspectiveCamera::setNearDistance(float nearDistance)
	{
		mNearDistance = nearDistance;
		mRevalidate = true;
	}

	void PerspectiveCamera::setFarDistance(float farDistance)
	{
		mFarDistance = farDistance;
		mRevalidate = true;
	}

	void PerspectiveCamera::setLook(glm::vec3 look)
	{		
		look = normalize(look);
		assertValidVector(look);

		mCoordSystem.look = std::move(look);		
		mRevalidate = true;
	}

	void PerspectiveCamera::setPosition(glm::vec3 position)
	{
		mCoordSystem.position = std::move(position);
		mRevalidate = true;
	}

	void PerspectiveCamera::setUp(glm::vec3 up)
	{
		up = normalize(up);
		assertValidVector(up);

		mCoordSystem.up = std::move(up);
		mRevalidate = true;
	}

	void PerspectiveCamera::update(bool updateAlways)
	{
		if (mRevalidate || updateAlways)
		{
			mRight = normalize(cross(mCoordSystem.look, mCoordSystem.up));
			mProjection = glm::perspective(radians(mFovY),
				mAspectRatio, mNearDistance, mFarDistance);

			calcFrustum();
			calcView();
			mRevalidate = false;
		}
	}

	void PerspectiveCamera::assertValidVector(const glm::vec3& vec)
	{
		if (isnan(vec.x) ||
			isnan(vec.y) ||
			isnan(vec.z))
		{
			throw_with_trace(runtime_error("PerspectiveProjectional::assertValidVector: specified a non valid vector!"));
		}
	}

	void PerspectiveCamera::calcFrustum()
	{
		const auto zNear = getViewSpaceZfromDistance(mNearDistance);
		const auto zFar = getViewSpaceZfromDistance(mFarDistance);


		const auto halfFovY = radians(mFovY/2.0f);

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
}