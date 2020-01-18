#include <nex/math/TrafoSpace.hpp>

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <nex/math/Math.hpp>
#include <nex/common/Log.hpp>
#include <nex/util/ExceptionHandling.hpp>

void nex::TrafoSpace::compose()
{
	if (!mNeedsCompose) return;

	const auto unitMatrix = glm::mat4(1.0f);
	const auto rotationMat = glm::toMat4(mRotation);
	const auto scaleMat = glm::scale(unitMatrix, mScale);
	const auto transMat = translate(unitMatrix, mPosition);

	auto shearXY = unitMatrix; shearXY[1][0] = mShear.z;
	auto shearYZ = unitMatrix; shearYZ[2][1] = mShear.x;
	auto shearXZ = unitMatrix; shearXZ[2][0] = mShear.y;
	auto shearMatrix = shearYZ * shearXZ * shearXY;

	mTrafo = transMat * rotationMat * shearMatrix * scaleMat;

	mNeedsCompose = false;
}

void nex::TrafoSpace::decompose()
{
	if (!mNeedsDecompose) return;

	// We use temporary variables since we have to check for numeric integrity
	glm::vec3 scaleT, positionT, skewT;
	glm::quat rotationT;
	glm::vec4 perspectiveT;

	glm::decompose(mTrafo, scaleT, rotationT, positionT, skewT, perspectiveT);

	if (nex::isValid(scaleT)) {
		mScale = scaleT;
	}

	if (nex::isValid(skewT)) {
		mShear = skewT;
	}

	if (nex::isValid(rotationT)) {
		mRotation = rotationT;
	}

	if (nex::isValid(positionT)) {
		mPosition = positionT;
	}

	if (nex::isValid(perspectiveT)) {
		mPerspective = perspectiveT;
	}

	mNeedsDecompose = false;
}

void nex::TrafoSpace::assertSyncState(bool assertion) const
{
#ifndef EUCLID_ALL_OPTIMIZATIONS
	if (!assertion) {
		throw_with_trace(std::runtime_error("Out of synchronization!"));
	}
#endif
}

bool nex::TrafoSpace::checkUpdateState() const noexcept
{
	return mNeedsCompose || mNeedsDecompose;
}

glm::vec3 nex::TrafoSpace::calcScaleFromTrafo() const
{
	return glm::vec3(length(mTrafo[0]), length(mTrafo[1]), length(mTrafo[2]));
}

const glm::vec4& nex::TrafoSpace::getPerspective() const
{
	assertSyncState(!mNeedsDecompose);
	return mPerspective;
}

const glm::vec3& nex::TrafoSpace::getPosition() const
{
	assertSyncState(!mNeedsDecompose);
	return mPosition;
}

const glm::quat& nex::TrafoSpace::getRotation() const
{
	assertSyncState(!mNeedsDecompose);
	return mRotation;
}

const glm::vec3& nex::TrafoSpace::getScale() const
{
	assertSyncState(!mNeedsDecompose);
	return mScale;
}

const glm::vec3& nex::TrafoSpace::getShear() const
{
	assertSyncState(!mNeedsDecompose);
	return mShear;
}

const glm::mat4& nex::TrafoSpace::getTrafo() const
{
	assertSyncState(!mNeedsCompose);
	return mTrafo;
}

void nex::TrafoSpace::setPerspective(const glm::vec4& vec)
{
	assertSyncState(!mNeedsDecompose);
	mPerspective = vec;
	mNeedsCompose = true;
}

void nex::TrafoSpace::setPosition(const glm::vec3& vec)
{
	// Note: position needs no composition/decomposition state check;
	// We can easily update it.
	mPosition = vec;
	mNeedsCompose = true;
	mTrafo[3] = glm::vec4(mPosition, 1.0f);
}

void nex::TrafoSpace::setRotation(const glm::quat& q)
{
	assertSyncState(!mNeedsDecompose);
	mRotation = q;
	mNeedsCompose = true;
}

void nex::TrafoSpace::setScale(const glm::vec3& vec)
{
	assertSyncState(!mNeedsDecompose);
	mScale = vec;
	mNeedsCompose = true;
}

void nex::TrafoSpace::setShear(const glm::vec3& vec)
{
	assertSyncState(!mNeedsDecompose);
	mShear = vec;
	mNeedsCompose = true;
}

void nex::TrafoSpace::setTrafo(const glm::mat4& mat)
{
	assertSyncState(!mNeedsCompose);
	mTrafo = mat;
	mNeedsDecompose = true;
}

void nex::TrafoSpace::update()
{
#ifndef EUCLID_ALL_OPTIMIZATIONS 
	if (mNeedsCompose && mNeedsDecompose) {
		LOG(nex::Logger("TrafoSpace"), Warning) << "compose and decompose are both needed. That indicates a possible bug!";
	}
#endif

	compose();
	decompose();
}