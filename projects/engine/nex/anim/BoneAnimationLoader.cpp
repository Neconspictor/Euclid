#include <nex/anim/BoneAnimationLoader.hpp>
#include <nex/util/StringUtils.hpp>
#include <nex/exception/ResourceLoadException.hpp>
#include <nex/import/ImportScene.hpp>

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
	boneAni.setTicks(ani->mDuration);
	boneAni.setTicksPerSecond(ani->mTicksPerSecond);
	boneAni.setRig(rig);

	for (auto j = 0; j < ani->mNumChannels; ++j) {
		loadBoneChannel(boneAni, ani->mChannels[j]);
	}

	return std::make_unique<BoneAnimation>(boneAni);
}

void nex::BoneAnimationLoader::loadBoneChannel(BoneAnimationData& boneAni, aiNodeAnim* nodeAni)
{
	auto nodeName = nodeAni->mNodeName;
	auto sid = SID(nodeName.C_Str());

	static_assert(sizeof(aiVector3D) == sizeof(glm::vec3), "aiVector3D and glm::vec3 don't match i size");
	static_assert(sizeof(aiQuaternion) == sizeof(glm::quat), "aiQuaternion and glm::quat don't match i size");
	
	//positions
	for (int i = 0; i < nodeAni->mNumPositionKeys; ++i) {
		const auto& key = nodeAni->mPositionKeys[i];
		const auto& p = key.mValue;
		boneAni.addPositionKey({ sid, static_cast<float>(key.mTime), nex::ImportScene::convert(p)});
	}

	// rotations
	for (int i = 0; i < nodeAni->mNumRotationKeys; ++i) {
		const auto& key = nodeAni->mRotationKeys[i];
		const auto& q = key.mValue;
		boneAni.addRotationKey({ sid, static_cast<float>(key.mTime),  nex::ImportScene::convert(q) });
	}

	// scalings
	for (int i = 0; i < nodeAni->mNumScalingKeys; ++i) {
		const auto& key = nodeAni->mScalingKeys[i];
		const auto& s = key.mValue;
		boneAni.addScaleKey({ sid, static_cast<float>(key.mTime),  nex::ImportScene::convert(s) });
	}
}