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

void nex::BoneAnimationData::setName(const std::string& name)
{
	mName = name;
}

void nex::BoneAnimationData::setRig(const Rig* rig)
{
	mRig = rig;
}

void nex::BoneAnimationData::setFrameCount(float frameCount)
{
	mFrameCount = frameCount;
}

void nex::BoneAnimationData::setFramesPerSecond(float framesPerSecond)
{
	mFramesPerSecond = framesPerSecond;
}

void nex::BoneAnimationData::addPositionKey(KeyFrame<glm::vec3, Sid> keyFrame)
{
	mPositionKeys.emplace_back(std::move(keyFrame));
}

void nex::BoneAnimationData::addRotationKey(KeyFrame<glm::quat, Sid> keyFrame)
{
	mRotationKeys.emplace_back(std::move(keyFrame));
}

void nex::BoneAnimationData::addScaleKey(KeyFrame<glm::vec3, Sid> keyFrame)
{
	mScaleKeys.emplace_back(std::move(keyFrame));
}

float nex::BoneAnimationData::getFrameCount() const
{
	return mFrameCount;
}


nex::BoneAnimation::BoneAnimation(const BoneAnimationData& data)
{

	if (data.mRig == nullptr) throw_with_trace(std::invalid_argument("nex::BoneAnimation : rig mustn't be null!"));

	mName = data.mName;
	mRigID = data.mRig->getID();
	mRigSID = data.mRig->getSID();
	mFrameCount = data.mFrameCount;
	mFramesPerSecond = data.mFramesPerSecond;

	auto* rig = getRig();

	if (rig == nullptr) throw_with_trace(std::runtime_error("nex::BoneAnimation : rig from rig sid mustn't be null! Fix that bug!"));
	
	mBoneCount= static_cast<unsigned>(rig->getBones().size());

	// it is faster to resize first and than add elems by index.
	mRotations.reserve(data.mRotationKeys.size());
	mScales.reserve(data.mScaleKeys.size());

	// at first convert the sids to bone ids
	std::vector<KeyFrame<glm::vec3, BoneID>> positionKeysBoneID(data.mPositionKeys.size());
	std::vector<KeyFrame<glm::quat, BoneID>> rotationKeysBoneID(data.mRotationKeys.size());
	std::vector<KeyFrame<glm::vec3, BoneID>> scaleKeysBoneID(data.mScaleKeys.size());

	for (int i = 0; i < data.mPositionKeys.size(); ++i) {
		const auto& key = data.mPositionKeys[i];
		auto* bone = rig->getBySID(key.id);
		positionKeysBoneID[i] = { bone->getID(), key.frame, key.data };
	}

	for (int i = 0; i < data.mRotationKeys.size(); ++i) {
		const auto& key = data.mRotationKeys[i];
		auto* bone = rig->getBySID(key.id);
		rotationKeysBoneID[i] = { bone->getID(), key.frame, key.data };
	}

	for (int i = 0; i < data.mScaleKeys.size(); ++i) {
		const auto& key = data.mScaleKeys[i];
		auto* bone = rig->getBySID(key.id);
		scaleKeysBoneID[i] = { bone->getID(), key.frame, key.data };
	}

	// now extend/interpolate trafos 
	createInterpolations(positionKeysBoneID, mPositions, static_cast<int>(mFrameCount), mBoneCount);
	createInterpolations(rotationKeysBoneID, mRotations, static_cast<int>(mFrameCount), mBoneCount);
	createInterpolations(scaleKeysBoneID, mScales, static_cast<int>(mFrameCount), mBoneCount);
}

nex::MixData<int> nex::BoneAnimation::calcFrameMix(float animationTime) const
{
	const auto floatingFrame = animationTime * mFramesPerSecond;
	const auto minFrame = static_cast<int>(std::truncf(floatingFrame));
	const auto maxFrame = static_cast<int>(std::min<float>(minFrame + 1, mFrameCount - 1));

	return { minFrame, maxFrame, floatingFrame - minFrame };
}

void nex::BoneAnimation::calcBoneTrafo(float animationTime, std::vector<glm::mat4>& vec) const
{
	auto mixData = calcFrameMix(animationTime);
	const auto& minFrame = mixData.minData;
	const auto& maxFrame = mixData.maxData;
	const auto& ratio = mixData.ratio;


	const glm::mat4 unit(1.0f);
	if (vec.size() != mBoneCount) vec.resize(mBoneCount);

	for (int i = 0; i < vec.size(); ++i) {

		const auto minIndex = minFrame * mBoneCount + i;
		const auto maxIndex = maxFrame * mBoneCount + i;

		const auto positionData = glm::mix(mPositions[minIndex], mPositions[maxIndex], ratio);
		const auto rotationData = glm::slerp(mRotations[minIndex], mRotations[maxIndex], ratio);
		const auto scaleData = glm::mix(mScales[minIndex], mScales[maxIndex], ratio);

		const auto rotation = glm::toMat4(rotationData);
		const auto scale = glm::scale(unit, scaleData);
		const auto trans = glm::translate(unit, positionData);
		vec[i] = trans * rotation * scale;
	}
}

void nex::BoneAnimation::applyParentHierarchyTrafos(std::vector<glm::mat4>& vec) const
{
	auto* rig = getRig();
	const auto& bones = rig->getBones();
	const auto& invRootTrafo = rig->getInverseRootTrafo();
	auto rootTrafo = inverse(invRootTrafo);

	if (vec.size() != rig->getBones().size()) {
		throw_with_trace(std::invalid_argument(
			"nex::BoneAnimation::applyParentHierarchyTrafos : Matrix vector argument has to have the same size like there are bones!"));
	}

	static const std::function<void(const Bone*, const glm::mat4&)> recursive = [&](const Bone* bone, const glm::mat4& parentTrafo) {

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

const std::string& nex::BoneAnimation::getName() const
{
	return mName;
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

float nex::BoneAnimation::getFrameCount() const
{
	return mFrameCount;
}

float nex::BoneAnimation::getFramesPerSecond() const
{
	return mFramesPerSecond;
}

float nex::BoneAnimation::getDuration() const
{
	return mFrameCount / mFramesPerSecond;
}

void nex::BoneAnimation::write(nex::BinStream& out, const BoneAnimation& ani)
{

	static_assert(std::is_trivially_copyable_v<nex::KeyFrame<glm::vec3, SID>>, "");
	static_assert(std::is_trivially_copyable_v<nex::KeyFrame<glm::quat, SID>>, "");

	out << ani.mName;
	out << ani.mRigID;
	out << ani.mRigSID;
	out << ani.mFrameCount;
	out << ani.mBoneCount;
	out << ani.mFramesPerSecond;
	out << ani.mPositions;
	out << ani.mRotations;
	out << ani.mScales;
}

void nex::BoneAnimation::load(nex::BinStream& in, BoneAnimation& ani)
{
	in >> ani.mName;
	in >> ani.mRigID;
	in >> ani.mRigSID;
	in >> ani.mFrameCount;
	in >> ani.mBoneCount;
	in >> ani.mFramesPerSecond;
	in >> ani.mPositions;
	in >> ani.mRotations;
	in >> ani.mScales;
}

nex::BoneAnimation nex::BoneAnimation::createUnintialized()
{
	return BoneAnimation();
}

int nex::BoneAnimation::getNextFrame(const std::vector<bool> flaggedInput, int frameCount, int boneCount, int boneID, int lastFrame)
{
	const auto lastIndex = lastFrame * boneCount + boneID;

	for (auto nextIndex = lastIndex + boneCount; nextIndex < flaggedInput.size(); nextIndex += boneCount) {
		if (flaggedInput[nextIndex]) return (nextIndex - boneID) / boneCount;
	}

	return lastFrame;
}

nex::BinStream& nex::operator>>(nex::BinStream& in, BoneAnimation& ani)
{
	BoneAnimation::load(in, ani);
	return in;
}

nex::BinStream& nex::operator<<(nex::BinStream& out, const BoneAnimation& ani)
{
	BoneAnimation::write(out, ani);
	return out;
}