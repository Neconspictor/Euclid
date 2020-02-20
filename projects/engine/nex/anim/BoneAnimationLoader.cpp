#include <nex/anim/BoneAnimationLoader.hpp>
#include <nex/util/StringUtils.hpp>
#include <nex/exception/ResourceLoadException.hpp>
#include <nex/import/ImportScene.hpp>
#include <nex/anim/Rig.hpp>


std::unique_ptr<nex::BoneAnimation> nex::BoneAnimationLoader::load(const aiScene* scene, const Rig* rig, const std::string& aniName)
{
	if (scene->mNumAnimations != 1) {
		throw_with_trace(nex::ResourceLoadException("scene is expected to have exact one animation!"));
	}

	if (!rig) {
		throw_with_trace(nex::ResourceLoadException("Rig mustn't be null!"));
	}

	aiAnimation* ani = scene->mAnimations[0];

	if (ani->mNumChannels == 0)
		throw_with_trace(nex::ResourceLoadException("Animation is expected to have at least one channel!"));

	const float samplingFrequency = 1.0f;

	BoneAnimationData boneAni;
	boneAni.setName(aniName);

	/**
	 * Documentation for assimp bug in glTF2Importer.cpp:
	 * Note to assimp: Tick is not specified, but I assume it is the time difference between two key frames measured in milliseconds.
	 * For an animation there are 'frame count minus one' ticks. Thus: tick_count = frame_count - 1
	 * aiAnimation.mDuration = (tick_count) * (one_tick)
	 * aiAnimation.mTicksPerSecond = frame_count * 1000 / duration   (from code/glTF2/glTF2Importer.cpp)
	 * Thus mTicksPerSecond is NOT the number of ticks per second (as its name would suggest), 
	 * but it is 'real_ticks_per_second * frame_count / (frame_count - 1)'.
	 * => real_ticks_per_seconds = mTicksPerSecond * (frame_count - 1) / frame_count
	 * To get the real ticks per seconds you have to retrieve the frame count:
	 * frame_count = mTicksPerSecond * duration / 1000
	 * tick = mDuration / (frame_count - 1)
	 * frames_per_seconds = frame_count / tick
	 * To retrieve the frame index by a time value, just divide the time by tick : time / tick
	 */


	// Note we round due to limited floating point precision. The result should be an integer.
	const float frameCount = std::roundf(ani->mTicksPerSecond * ani->mDuration / 1000.0f);
	const float tickCount = frameCount > 0 ? frameCount - 1 : 0;
	const float tick = tickCount > 0 ? ani->mDuration / tickCount : 0.0f;
	const float realTicksPerSecond = std::roundf(ani->mTicksPerSecond * tickCount / frameCount);
	const float framesPerSecond = realTicksPerSecond + 1;


	boneAni.setTickCount(std::roundf(ani->mDuration));
	boneAni.setTicksPerSecond(std::roundf(ani->mTicksPerSecond)); //ani->mTicksPerSecond ani->mTicksPerSecond * 
	boneAni.setRig(rig);

	for (auto j = 0; j < ani->mNumChannels; ++j) {
		loadBoneChannel(boneAni, ani->mChannels[j], rig);
	}

	return std::make_unique<BoneAnimation>(boneAni);
}

void nex::BoneAnimationLoader::loadBoneChannel(BoneAnimationData& boneAni, aiNodeAnim* nodeAni, const Rig* rig)
{
	auto nodeName = nodeAni->mNodeName;
	auto sid = SID(nodeName.C_Str());	

	//positions
	for (int i = 0; i < nodeAni->mNumPositionKeys; ++i) {
		const auto& key = nodeAni->mPositionKeys[i];
		const auto& p = key.mValue;
		const int index = static_cast<int>(std::roundf(key.mTime));
		boneAni.addPositionKey({ sid, index, nex::ImportScene::convert(p)});
	}

	// rotations
	for (int i = 0; i < nodeAni->mNumRotationKeys; ++i) {
		const auto& key = nodeAni->mRotationKeys[i];
		const auto& q = key.mValue;
		const int index = static_cast<int>(std::roundf(key.mTime));
		boneAni.addRotationKey({ sid, index,  nex::ImportScene::convert(q) });
	}

	// scalings
	for (int i = 0; i < nodeAni->mNumScalingKeys; ++i) {
		const auto& key = nodeAni->mScalingKeys[i];
		const auto& s = key.mValue;
		const int index = static_cast<int>(std::roundf(key.mTime));
		boneAni.addScaleKey({ sid, index,  nex::ImportScene::convert(s) });
	}
}