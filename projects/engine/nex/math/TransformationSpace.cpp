#include <nex/math/TransformationSpace.hpp>

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <nex/math/Math.hpp>
#include <nex/common/Log.hpp>

void nex::SpaceTrafo::compose()
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

void nex::SpaceTrafo::decompose()
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

glm::vec3 nex::SpaceTrafo::calcScaleFromTrafo() const
{
	return glm::vec3(length(mTrafo[0]), length(mTrafo[1]), length(mTrafo[2]));
}

const glm::vec4& nex::SpaceTrafo::getPerspective() const
{
	return mPerspective;
}

const glm::vec3& nex::SpaceTrafo::getPosition() const
{
	return mPosition;
}

const glm::quat& nex::SpaceTrafo::getRotation() const
{
	return mRotation;
}

const glm::vec3& nex::SpaceTrafo::getScale() const
{
	return mScale;
}

const glm::vec3& nex::SpaceTrafo::getShear() const
{
	return mShear;
}

const glm::mat4& nex::SpaceTrafo::getTrafo() const
{
	return mTrafo;
}

void nex::SpaceTrafo::setPerspective(const glm::vec4& vec)
{
	mPerspective = vec;
	mNeedsCompose = true;
}

void nex::SpaceTrafo::setPosition(const glm::vec3& vec)
{
	mPosition = vec;
	mNeedsCompose = true;
}

void nex::SpaceTrafo::setRotation(const glm::quat& q)
{
	mRotation = q;
	mNeedsCompose = true;
}

void nex::SpaceTrafo::setScale(const glm::vec3& vec)
{
	mScale = vec;
	mNeedsCompose = true;
}

void nex::SpaceTrafo::setShear(const glm::vec3& vec)
{
	mShear = vec;
	mNeedsCompose = true;
}

void nex::SpaceTrafo::setTrafo(const glm::mat4& mat)
{
	mTrafo = mat;
	mNeedsDecompose = true;
}

void nex::SpaceTrafo::update()
{
#ifndef EUCLID_ALL_OPTIMIZATIONS 
	if (mNeedsCompose && mNeedsDecompose) {
		LOG(nex::Logger("SpaceTrafo"), Warning) << "compose and decompose are both needed. That indicates a possible bug!";
	}
#endif

	compose();
	decompose();
}