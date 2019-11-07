#include <nex/anim/BoneAnimation.hpp>
#include <nex/anim/Rig.hpp>
#include <nex/util/ExceptionHandling.hpp>

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
	mPositionKeys.insert(std::move(keyFrame));
}

void nex::BoneAnimationData::addRotationKey(RotationKeyFrame<SID> keyFrame)
{
	mRotationKeys.insert(std::move(keyFrame));
}

void nex::BoneAnimationData::addScaleKey(ScaleKeyFrame<SID> keyFrame)
{
	mScaleKeys.insert(std::move(keyFrame));
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