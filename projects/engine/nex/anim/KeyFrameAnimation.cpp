#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#include <nex/anim/KeyFrameAnimation.hpp>
#include <nex/anim/Rig.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <algorithm>
#include <nex/math/Math.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <nex/anim/AnimationManager.hpp>
#include <functional>


void nex::KeyFrameAnimationData::setName(const std::string& name)
{
	mName = name;
}

void nex::KeyFrameAnimationData::setTickCount(float tickCount)
{
	mTickCount = tickCount;
}

void nex::KeyFrameAnimationData::setTicksPerSecond(float framesPerSecond)
{
	mTicksPerSecond = framesPerSecond;
}

void nex::KeyFrameAnimationData::addPositionKey(KeyFrame<glm::vec3, Sid> keyFrame)
{
	mPositionKeys.emplace_back(std::move(keyFrame));
}

void nex::KeyFrameAnimationData::addRotationKey(KeyFrame<glm::quat, Sid> keyFrame)
{
	mRotationKeys.emplace_back(std::move(keyFrame));
}

void nex::KeyFrameAnimationData::addScaleKey(KeyFrame<glm::vec3, Sid> keyFrame)
{
	mScaleKeys.emplace_back(std::move(keyFrame));
}

unsigned nex::KeyFrameAnimationData::getChannelCount() const
{
	return mChannelCount;
}

float nex::KeyFrameAnimationData::getTickCount() const
{
	return mTickCount;
}

void nex::KeyFrameAnimationData::setChannelCount(unsigned channelCount)
{
	mChannelCount = channelCount;
}




nex::KeyFrameAnimation::KeyFrameAnimation(const KeyFrameAnimationData& data)
{
	mName = data.mName;
	mTickCount = data.mTickCount;
	mTicksPerSecond = data.mTicksPerSecond;

	mChannelCount = mChannelCount;
}

nex::KeyFrameAnimation::KeyFrameAnimation(const KeyFrameAnimationData& data, const ChannelIDGenerator& generator) : 
	KeyFrameAnimation(data)
{
	init(data, generator);
}


nex::MixData<int> nex::KeyFrameAnimation::calcFrameMix(float animationTime) const
{
	const auto floatingFrame = animationTime * mTicksPerSecond;
	const auto minFrame = static_cast<int>(std::truncf(floatingFrame));
	const auto maxFrame = static_cast<int>(std::min<float>(minFrame + 1, mTickCount));

	return { minFrame, maxFrame, floatingFrame - minFrame };
}

void nex::KeyFrameAnimation::calcChannelTrafos(float animationTime, std::vector<glm::mat4>& vec) const
{
	auto mixData = calcFrameMix(animationTime);
	const auto& maxFrame = mixData.maxData;
	const auto& minFrame = std::min<float>(mixData.minData, mixData.maxData);

	const auto& ratio = mixData.ratio;


	const glm::mat4 unit(1.0f);
	if (vec.size() != mChannelCount) vec = mDefaultMatrices;

	for (int i = 0; i < vec.size(); ++i) {

		const auto minIndex = minFrame * mChannelCount + i;
		const auto maxIndex = maxFrame * mChannelCount + i;

		const auto positionData = glm::mix(mPositions[minIndex], mPositions[maxIndex], ratio);
		const auto rotationData = glm::slerp(mRotations[minIndex], mRotations[maxIndex], ratio);
		const auto scaleData = glm::mix(mScales[minIndex], mScales[maxIndex], ratio);

		const auto rotation = glm::toMat4(rotationData);
		const auto scale = glm::scale(unit, scaleData);
		const auto trans = glm::translate(unit, positionData);
		vec[i] = trans * rotation * scale * unit;
	}
}

unsigned nex::KeyFrameAnimation::getChannelCount() const
{
	return mChannelCount;
}

const std::vector<glm::mat4>& nex::KeyFrameAnimation::getDefaultMatrices() const
{
	return mDefaultMatrices;
}


const std::string& nex::KeyFrameAnimation::getName() const
{
	return mName;
}

float nex::KeyFrameAnimation::getTickCount() const
{
	return mTickCount;
}

float nex::KeyFrameAnimation::getFrameCount() const
{
	return mTickCount + 1;
}

float nex::KeyFrameAnimation::getTicksPerSecond() const
{
	return mTicksPerSecond;
}

float nex::KeyFrameAnimation::getDuration() const
{
	return mTickCount / mTicksPerSecond;
}

const std::unordered_set<nex::ChannelID>& nex::KeyFrameAnimation::getUsedChannelIDs() const
{
	return mUsedChannelIDs;
}

void nex::KeyFrameAnimation::write(nex::BinStream& out) const
{
	static_assert(std::is_trivially_copyable_v<nex::KeyFrame<glm::vec3, SID>>, "");
	static_assert(std::is_trivially_copyable_v<nex::KeyFrame<glm::quat, SID>>, "");

	out << mName;
	out << mTickCount;
	out << mChannelCount;
	out << mTicksPerSecond;
	out << mPositions;
	out << mRotations;
	out << mScales;
}

void nex::KeyFrameAnimation::load(nex::BinStream& in)
{
	in >> mName;
	in >> mTickCount;
	in >> mChannelCount;
	in >> mTicksPerSecond;
	in >> mPositions;
	in >> mRotations;
	in >> mScales;
}


void nex::KeyFrameAnimation::init(const KeyFrameAnimationData& data, const ChannelIDGenerator& generator)
{
	// it is faster to resize first and than add elems by index.
	mPositions.reserve(data.mPositionKeys.size());
	mRotations.reserve(data.mRotationKeys.size());
	
	// at first convert the sids to bone ids
	std::vector<KeyFrame<glm::vec3, ChannelID>> positionKeysBoneID(data.mPositionKeys.size());
	std::vector<KeyFrame<glm::quat, ChannelID>> rotationKeysBoneID(data.mRotationKeys.size());
	std::vector<KeyFrame<glm::vec3, ChannelID>> scaleKeysBoneID(data.mScaleKeys.size());

	mUsedChannelIDs.clear();

	for (int i = 0; i < data.mPositionKeys.size(); ++i) {
		const auto& key = data.mPositionKeys[i];
		//auto* bone = rig->getBySID(key.id);
		const auto id = generator(key.id);
		mUsedChannelIDs.insert(id);
		positionKeysBoneID[i] = { id, key.frame, key.data };
	}

	for (int i = 0; i < data.mRotationKeys.size(); ++i) {
		const auto& key = data.mRotationKeys[i];
		const auto id = generator(key.id);
		mUsedChannelIDs.insert(id);
		rotationKeysBoneID[i] = { id, key.frame, key.data };
	}

	for (int i = 0; i < data.mScaleKeys.size(); ++i) {
		const auto& key = data.mScaleKeys[i];
		const auto id = generator(key.id);
		mUsedChannelIDs.insert(id);
		scaleKeysBoneID[i] = { id, key.frame, key.data };
	}


	mChannelCount = data.getChannelCount();
	const int frameCount = static_cast<int>(getFrameCount());

	// now extend/interpolate trafos 

	const auto totalCount = frameCount * mChannelCount;
	createInterpolations(positionKeysBoneID, mPositions, frameCount, mChannelCount);
	createInterpolations(rotationKeysBoneID, mRotations, frameCount, mChannelCount);
	createInterpolations(scaleKeysBoneID, mScales, frameCount, mChannelCount);

	mDefaultMatrices.resize(mChannelCount);
	for (int i = 0; i < mChannelCount; ++i) {
		mDefaultMatrices[i] = glm::mat4(1.0f);
	}
}

int nex::KeyFrameAnimation::getNextFrame(const std::vector<bool> flaggedInput, int frameCount, int channelCount, int channelID, int lastFrame)
{
	const auto lastIndex = lastFrame * channelCount + channelID;

	for (auto nextIndex = lastIndex + channelCount; nextIndex < flaggedInput.size(); nextIndex += channelCount) {
		if (flaggedInput[nextIndex]) return (nextIndex - channelID) / channelCount;
	}

	return lastFrame;
}

nex::BinStream& nex::operator>>(nex::BinStream& in, KeyFrameAnimation& ani)
{
	ani.load(in);
	return in;
}

nex::BinStream& nex::operator<<(nex::BinStream& out, const KeyFrameAnimation& ani)
{
	ani.write(out);
	return out;
}