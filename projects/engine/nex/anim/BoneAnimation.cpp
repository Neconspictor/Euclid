#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#include <nex/anim/BoneAnimation.hpp>
#include <nex/anim/Rig.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <algorithm>
#include <nex/math/Math.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtx/quaternion.hpp>

void nex::BoneAnimationData::setName(const std::string& name)
{
	mName = name;
}

void nex::BoneAnimationData::setRig(const Rig* rig)
{
	mRig = rig;
}

void nex::BoneAnimationData::setTicks(float ticks)
{
	mTicks = ticks;
}

void nex::BoneAnimationData::setTicksPerSecond(float ticksPerSecond)
{
	mTicksPerSecond = ticksPerSecond;
}

void nex::BoneAnimationData::addPositionKey(PositionKeyFrame<SID> keyFrame)
{
	mPositionKeys.emplace_back(std::move(keyFrame));
}

void nex::BoneAnimationData::addRotationKey(RotationKeyFrame<SID> keyFrame)
{
	mRotationKeys.emplace_back(std::move(keyFrame));
}

void nex::BoneAnimationData::addScaleKey(ScaleKeyFrame<SID> keyFrame)
{
	mScaleKeys.emplace_back(std::move(keyFrame));
}


nex::BoneAnimation::BoneAnimation(const BoneAnimationData& data)
{

	if (data.mRig == nullptr) throw_with_trace(std::invalid_argument("nex::BoneAnimation : rig mustn't be null!"));

	mName = data.mName;
	mRig = data.mRig;
	mTicks = data.mTicks;
	mTicksPerSecond = data.mTicksPerSecond;
	
	// it is faster to resize first and than add elems by index.
	mPositions.reserve(data.mPositionKeys.size());
	mRotations.reserve(data.mRotationKeys.size());
	mScales.reserve(data.mScaleKeys.size());
	int i = 0;

	for (const auto& key : data.mPositionKeys) {
		auto* bone = mRig->getBySID(key.id);
		mPositions[i] = { bone->getID(), key.time, key.position };
		++i;
	}

	i = 0;
	for (const auto& key : data.mRotationKeys) {
		auto* bone = mRig->getBySID(key.id);
		mRotations[i] = { bone->getID(), key.time, key.rotation };
		++i;
	}

	i = 0;
	for (const auto& key : data.mScaleKeys) {
		auto* bone = mRig->getBySID(key.id);
		mScales[i] = { bone->getID(), key.time, key.scale };
		++i;
	}

	// We sort by bone ids, so that finding suitable min/max keyframes 
	// for animation time is easier
	constexpr auto cmp = nex::KeyFrame<BoneID>::Comparator();
	std::sort(mPositions.begin(), mPositions.end(), cmp);
	std::sort(mRotations.begin(), mRotations.end(), cmp);
	std::sort(mScales.begin(), mScales.end(), cmp);
}

std::vector<nex::MinMaxKeyFrame> nex::BoneAnimation::calcMinMaxKeyFrames(float time) const
{
	const auto boneSize = mRig->getBones().size();
	std::vector<MinMaxKeyFrame> keys(boneSize);

	// Go over each keyframe type and find min and max.
	// We only retrieve the key frame ids to avoid unnecessary data copying.
	unsigned id = UINT_MAX;
	for (int i = 0; i < mPositions.size(); ++i) {
		const auto data = mPositions[i];
		calcMinMaxKeyId(keys, offsetof(MinMaxKeyFrame, MinMaxKeyFrame::positions), time, id, data.time, i, data.id);
	}

	// Important: reset id!
	id = UINT_MAX;
	for (int i = 0; i < mRotations.size(); ++i) {
		const auto data = mRotations[i];
		calcMinMaxKeyId(keys, offsetof(MinMaxKeyFrame, MinMaxKeyFrame::rotations), time, id, data.time, i, data.id);
	}

	// Important: reset id!
	id = UINT_MAX;
	for (int i = 0; i < mScales.size(); ++i) {
		const auto data = mScales[i];
		calcMinMaxKeyId(keys, offsetof(MinMaxKeyFrame, MinMaxKeyFrame::scales), time, id, data.time, i, data.id);
	}

	// copy key frame data
	for (int i = 0; i < boneSize; ++i) {
		auto& positions = keys[i].positions;
		auto& rotations = keys[i].rotations;
		auto& scales = keys[i].scales;

		positions.minData = mPositions[positions.minKeyID].position;
		positions.maxData = mPositions[positions.maxKeyID].position;

		rotations.minData = mRotations[rotations.minKeyID].rotation;
		rotations.maxData = mRotations[rotations.maxKeyID].rotation;

		scales.minData = mScales[scales.minKeyID].scale;
		scales.maxData = mScales[scales.maxKeyID].scale;
	}

	return keys;
}

std::vector<nex::CompoundKeyFrame> nex::BoneAnimation::calcInterpolatedTrafo(const std::vector<MinMaxKeyFrame>& minMaxs, 
	float animationTime) const
{
	std::vector<CompoundKeyFrame> keys(minMaxs.size());

	for (int i = 0; i < keys.size(); ++i)
	{
		const auto& minMax = minMaxs[i];
		const auto positionFactor = calcNormalizedInterpolationFactor(animationTime, 
			minMax.positions.minTime, minMax.positions.maxTime);

		const auto rotationFactor = calcNormalizedInterpolationFactor(animationTime,
			minMax.rotations.minTime, minMax.rotations.maxTime);

		const auto scaleFactor = calcNormalizedInterpolationFactor(animationTime,
			minMax.scales.minTime, minMax.scales.maxTime);

		auto& k = keys[i];

		k.position = glm::mix(minMax.positions.minData, minMax.positions.maxData, positionFactor);
		k.rotation = glm::mix(minMax.rotations.minData, minMax.rotations.maxData, rotationFactor);
		k.scale = glm::mix(minMax.scales.minData, minMax.scales.maxData, scaleFactor);

	}
	return keys;
}

std::vector<glm::mat4> nex::BoneAnimation::calcBoneTrafo(const std::vector<CompoundKeyFrame>& keyFrames) const
{
	std::vector<glm::mat4> vec(keyFrames.size());

	glm::mat4 unit(1.0f);

	for (int i = 0; i < vec.size(); ++i) {
		const auto& data = keyFrames[i];
		vec[i] = glm::toMat4(data.rotation) * glm::scale(unit, data.scale);
		vec[i] = glm::translate(vec[i], data.position);
	}

	return vec;
}

const std::string& nex::BoneAnimation::getName() const
{
	return mName;
}

const nex::Rig* nex::BoneAnimation::getRig() const
{
	return mRig;
}

float nex::BoneAnimation::getTicks() const
{
	return mTicks;
}

float nex::BoneAnimation::getTicksPerSecond() const
{
	return mTicksPerSecond;
}

float nex::BoneAnimation::getDuration() const
{
	return mTicks / mTicksPerSecond;
}

void nex::BoneAnimation::calcMinMaxKeyId(std::vector<nex::MinMaxKeyFrame>& keys, 
	size_t structOffset, 
	const float animationTime,  
	unsigned& boneID, 
	const float currentTime,
	const unsigned currentKeyID,
	const unsigned currentBoneID) const
{
	if (boneID != currentBoneID) {
		boneID = currentBoneID;
		auto& key = keys[boneID];

		// Note: We are only interested in time and key ids
		// -> it doesn't matter that we cast to a wrong class type
		auto& keyBaseData = *(MinMaxData<char>*)(((const char*)&key) + structOffset);

		keyBaseData.minTime = currentTime;
		keyBaseData.minKeyID = currentKeyID;
		keyBaseData.maxTime = currentTime;
		keyBaseData.maxKeyID = currentKeyID;
	
	} else {

		auto& key = keys[boneID];

		// Note: We are only interested in time and key id
		// -> it doesn't matter that we cast to a wrong class type
		auto& keyBaseData = *(MinMaxData<char>*)(((const char*)&key) + structOffset);
		auto& minTime = keyBaseData.minTime;
		auto& minID = keyBaseData.minKeyID;
		auto& maxTime = keyBaseData.maxTime;
		auto& maxID = keyBaseData.maxKeyID;

		if ((currentTime <= animationTime) && (currentTime >= minTime)) {
			minTime = currentTime;
			minID = currentKeyID;
		}

		if ((currentTime >= animationTime) && (currentTime <= maxTime) || (maxTime < animationTime)) {
			maxTime = currentTime;
			maxID = currentKeyID;
		}
	}
}