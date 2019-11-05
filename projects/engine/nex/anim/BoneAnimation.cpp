#include <nex/anim/BoneAnimation.hpp>
#include <nex/anim/Rig.hpp>
#include <nex/util/ExceptionHandling.hpp>

const std::string& nex::BoneAnimation::getName() const
{
	return mName;
}

void nex::BoneAnimation::setName(const std::string& name)
{
	mName = name;
}

const std::vector<nex::OptPositionKeyFrame>& nex::BoneAnimation::getOptPositionKeys() const
{
	assert(mOptimized);
	return mPositionsOpt;
}

const std::vector<nex::OptRotationKeyFrame>& nex::BoneAnimation::getOptRotationKeys() const
{
	assert(mOptimized);
	return mRotationsOpt;
}

const std::vector<nex::OptScaleKeyFrame>& nex::BoneAnimation::getOptScaleKeys() const
{
	assert(mOptimized);
	return mScalesOpt;
}

const nex::Rig* nex::BoneAnimation::getRig() const
{
	return mRig;
}

nex::Rig* nex::BoneAnimation::getRig()
{
	return mRig;
}

double nex::BoneAnimation::getTicks() const
{
	return mTicks;
}

void nex::BoneAnimation::setTicks(double ticks)
{
	mTicks = ticks;
}

double nex::BoneAnimation::getTicksPerSecond() const
{
	return mTicksPerSecond;
}

void nex::BoneAnimation::setTicksPerSecond(double ticksPerSecond)
{
	mTicksPerSecond = ticksPerSecond;
}

double nex::BoneAnimation::getDuration() const
{
	return mTicks / mTicksPerSecond;
}

void nex::BoneAnimation::optimize(Rig* rig)
{
	if (rig == nullptr) throw_with_trace(std::invalid_argument("nex::BoneAnimation : rig mustn't be null!"));
	if (mOptimized) throw_with_trace(std::invalid_argument("nex::BoneAnimation : already optimized!"));

	mPositionsOpt.resize(mPositionKeys.size());
	mRotationsOpt.resize(mRotationKeys.size());
	mScalesOpt.resize(mScaleKeys.size());

	// it is faster to resize first and than add elems by index.
	int i = 0;

	for (const auto& key : mPositionKeys) {
		auto* bone = rig->getBySID(key.boneSID);
		mPositionsOpt[i] = {bone, key.time, key.position};
		++i;
	}

	i = 0;
	for (const auto& key : mRotationKeys) {
		auto* bone = rig->getBySID(key.boneSID);
		mRotationsOpt[i] = { bone, key.time, key.rotation };
		++i;
	}

	i = 0;
	for (const auto& key : mScaleKeys) {
		auto* bone = rig->getBySID(key.boneSID);
		mScalesOpt[i] = { bone, key.time, key.scale };
		++i;
	}

	mPositionKeys.clear();
	mRotationKeys.clear();
	mScaleKeys.clear();

	mOptimized = true;
	mRig = rig;
}

void nex::BoneAnimation::addPositionKey(PositionKeyFrame keyFrame)
{
	mPositionKeys.insert(std::move(keyFrame));
}

void nex::BoneAnimation::addRotationKey(RotationKeyFrame keyFrame)
{
	mRotationKeys.insert(std::move(keyFrame));
}

void nex::BoneAnimation::addScaleKey(ScaleKeyFrame keyFrame)
{
	mScaleKeys.insert(std::move(keyFrame));
}