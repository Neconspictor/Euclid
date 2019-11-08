#include <nex/anim/BoneAnimationLoader.hpp>
#include <nex/util/StringUtils.hpp>

std::vector<nex::BoneAnimation> nex::BoneAnimationLoader::load(const aiScene* scene, const Rig* rig)
{
	std::vector<BoneAnimation> anims;

	for (auto i = 0; i < scene->mNumAnimations; ++i) {
		aiAnimation* ani = scene->mAnimations[i];

		// skip animation if there is no bone animation available
		if (ani->mNumChannels == 0)
			continue;

		BoneAnimationData boneAni;
		boneAni.setName(ani->mName.C_Str());
		boneAni.setTicks(ani->mDuration);
		boneAni.setTicksPerSecond(ani->mTicksPerSecond);
		boneAni.setRig(rig);

		for (auto j = 0; j < ani->mNumChannels; ++j) {
			loadBoneChannel(boneAni, ani->mChannels[j]);
		}

		anims.emplace_back(BoneAnimation(boneAni));
	}

	return anims;
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
		boneAni.addPositionKey({ sid, static_cast<float>(key.mTime), (const glm::vec3&)p });
	}

	// rotations
	for (int i = 0; i < nodeAni->mNumRotationKeys; ++i) {
		const auto& key = nodeAni->mRotationKeys[i];
		const auto& q = key.mValue;
		boneAni.addRotationKey({ sid, static_cast<float>(key.mTime), (const glm::quat&)q});
	}

	// scalings
	for (int i = 0; i < nodeAni->mNumScalingKeys; ++i) {
		const auto& key = nodeAni->mScalingKeys[i];
		const auto& s = key.mValue;
		boneAni.addScaleKey({ sid, static_cast<float>(key.mTime), (const glm::vec3&)s });
	}
}