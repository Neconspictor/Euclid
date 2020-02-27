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
#include <nex/anim/AnimationManager.hpp>
#include <functional>

void nex::BoneAnimationData::setRig(const Rig* rig)
{
	mRig = rig;
}


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
	if (vec.size() != mChannelCount) vec.resize(mChannelCount);

	for (int i = 0; i < vec.size(); ++i) {

		const auto minIndex = minFrame * mChannelCount + i;
		const auto maxIndex = maxFrame * mChannelCount + i;

		const auto positionData = glm::mix(mPositions[minIndex], mPositions[maxIndex], ratio);
		const auto rotationData = glm::slerp(mRotations[minIndex], mRotations[maxIndex], ratio);
		const auto scaleData = glm::mix(mScales[minIndex], mScales[maxIndex], ratio);

		const auto rotation = glm::toMat4(rotationData);
		const auto scale = glm::scale(unit, scaleData);
		const auto trans = glm::translate(unit, positionData);
		vec[i] = trans * rotation * scale;
	}
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
	mRotations.reserve(data.mRotationKeys.size());
	mScales.reserve(data.mScaleKeys.size());

	// at first convert the sids to bone ids
	std::vector<KeyFrame<glm::vec3, ChannelID>> positionKeysBoneID(data.mPositionKeys.size());
	std::vector<KeyFrame<glm::quat, ChannelID>> rotationKeysBoneID(data.mRotationKeys.size());
	std::vector<KeyFrame<glm::vec3, ChannelID>> scaleKeysBoneID(data.mScaleKeys.size());

	for (int i = 0; i < data.mPositionKeys.size(); ++i) {
		const auto& key = data.mPositionKeys[i];
		//auto* bone = rig->getBySID(key.id);
		positionKeysBoneID[i] = { generator(key.id), key.frame, key.data };
	}

	for (int i = 0; i < data.mRotationKeys.size(); ++i) {
		const auto& key = data.mRotationKeys[i];
		rotationKeysBoneID[i] = { generator(key.id), key.frame, key.data };
	}

	for (int i = 0; i < data.mScaleKeys.size(); ++i) {
		const auto& key = data.mScaleKeys[i];
		scaleKeysBoneID[i] = { generator(key.id), key.frame, key.data };
	}


	const int frameCount = static_cast<int>(getFrameCount());

	// now extend/interpolate trafos 
	createInterpolations(positionKeysBoneID, mPositions, frameCount, mChannelCount);
	createInterpolations(rotationKeysBoneID, mRotations, frameCount, mChannelCount);
	createInterpolations(scaleKeysBoneID, mScales, frameCount, mChannelCount);
}

int nex::KeyFrameAnimation::getNextFrame(const std::vector<bool> flaggedInput, int frameCount, int channelCount, int channelID, int lastFrame)
{
	const auto lastIndex = lastFrame * channelCount + channelID;

	for (auto nextIndex = lastIndex + channelCount; nextIndex < flaggedInput.size(); nextIndex += channelCount) {
		if (flaggedInput[nextIndex]) return (nextIndex - channelID) / channelCount;
	}

	return lastFrame;
}



nex::BoneAnimation::BoneAnimation(const BoneAnimationData& data) : KeyFrameAnimation(data)
{

	if (data.mRig == nullptr) throw_with_trace(std::invalid_argument("nex::BoneAnimation : rig mustn't be null!"));

	mRigID = data.mRig->getID();
	mRigSID = data.mRig->getSID();

	auto* rig = getRig();

	if (rig == nullptr) throw_with_trace(std::runtime_error("nex::BoneAnimation : rig from rig sid mustn't be null! Fix that bug!"));

	mChannelCount = static_cast<unsigned>(rig->getBones().size());

	struct BoneChannelIDGenerator : public ChannelIDGenerator {

		BoneChannelIDGenerator(const BoneAnimationData& data) : data(data) {}

		short operator()(unsigned keyFrameSID) const override {
			const auto* bone = data.mRig->getBySID(keyFrameSID);
			return bone->getID();
		}

		const BoneAnimationData& data;
	};

	init(data, BoneChannelIDGenerator(data));
}

void nex::BoneAnimation::applyParentHierarchyTrafos(std::vector<glm::mat4>& vec) const
{
	auto* rig = getRig();
	const auto& bones = rig->getBones();
	auto invRootTrafo = rig->getInverseRootTrafo();
	auto rootTrafo = inverse(invRootTrafo);// *glm::mat4(0.03f);
	//invRootTrafo = inverse(rootTrafo);

	if (vec.size() != rig->getBones().size()) {
		throw_with_trace(std::invalid_argument(
			"nex::BoneAnimation::applyParentHierarchyTrafos : Matrix vector argument has to have the same size like there are bones!"));
	}

	const std::function<void(const Bone*, const glm::mat4&)> recursive = [&](const Bone* bone, const glm::mat4& parentTrafo) {

		auto id = bone->getID();
		const auto& nodeTrafo = vec[id];
		const auto& offset = bone->getOffsetMatrix();

		auto trafo = parentTrafo * nodeTrafo;
		vec[id] = invRootTrafo * trafo * offset;
		
		// recursive propagation
		const auto& children = bone->getChildrenIDs();
		for (int i = 0; i < bone->getChildrenCount(); ++i) {
			const auto childID = children[i];
			recursive(&bones[childID], trafo);
		}
	};

	recursive(rig->getRoot(), rootTrafo);// glm::mat4(1.0f));
}

const nex::Rig* nex::BoneAnimation::getRig() const
{
	return nex::AnimationManager::get()->getBySID(mRigSID);
}

const std::string& nex::BoneAnimation::getRigID() const
{
	return mRigID;
}

unsigned nex::BoneAnimation::getRigSID() const
{
	return mRigSID;
}

nex::BoneAnimation nex::BoneAnimation::createUnintialized()
{
	return BoneAnimation();
}

void nex::BoneAnimation::write(nex::BinStream& out) const
{
	KeyFrameAnimation::write(out);
	out << mRigID;
	out << mRigSID;
}

void nex::BoneAnimation::load(nex::BinStream& in)
{
	KeyFrameAnimation::load(in);
	in >> mRigID;
	in >> mRigSID;
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

nex::BinStream& nex::operator>>(nex::BinStream& in, BoneAnimation& ani)
{
	ani.load(in);
	return in;
}

nex::BinStream& nex::operator<<(nex::BinStream& out, const BoneAnimation& ani)
{
	ani.write(out);
	return out;
}