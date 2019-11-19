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

	BoneAnimationData boneAni;
	boneAni.setName(aniName);
	// Note: Duration gives the highest frame number -> +1 needed
	boneAni.setFrameCount(ani->mDuration + 1);
	boneAni.setFramesPerSecond(ani->mTicksPerSecond);
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
		boneAni.addPositionKey({ sid, static_cast<int>(key.mTime), nex::ImportScene::convert(p)});
	}

	// rotations
	for (int i = 0; i < nodeAni->mNumRotationKeys; ++i) {
		const auto& key = nodeAni->mRotationKeys[i];
		const auto& q = key.mValue;
		boneAni.addRotationKey({ sid, static_cast<int>(key.mTime),  nex::ImportScene::convert(q) });
	}

	// scalings
	for (int i = 0; i < nodeAni->mNumScalingKeys; ++i) {
		const auto& key = nodeAni->mScalingKeys[i];
		const auto& s = key.mValue;
		boneAni.addScaleKey({ sid, static_cast<int>(key.mTime),  nex::ImportScene::convert(s) });
	}
}