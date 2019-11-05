#include <nex/anim/BoneAnimationLoader.hpp>

std::vector<nex::BoneAnimation> nex::BoneAnimationLoader::load(const aiScene* scene)
{
	std::vector<BoneAnimation> anims;

	for (auto i = 0; i < scene->mNumAnimations; ++i) {
		aiAnimation* ani = scene->mAnimations[i];

		// skip animation if there is no bone animation available
		if (ani->mNumChannels == 0)
			continue;

		BoneAnimation boneAni;
		boneAni.setName(ani->mName.C_Str());
		boneAni.setTicks(ani->mDuration);
		boneAni.setTicksPerSecond(ani->mTicksPerSecond);

		for (auto j = 0; j < ani->mNumChannels; ++j) {
			loadBoneChannel(boneAni, ani->mChannels[j]);
		}

		anims.emplace_back(std::move(boneAni));
	}

	return anims;
}

void nex::BoneAnimationLoader::loadBoneChannel(BoneAnimation& boneAni, aiNodeAnim* nodeAni)
{
	auto nodeName = nodeAni->mNodeName;
	
	//positions
	for (int i = 0; i < nodeAni->mNumPositionKeys; ++i) {
		const auto& key = nodeAni->mPositionKeys[i];
		//key.
	}

	// rotations
	for (int i = 0; i < nodeAni->mNumPositionKeys; ++i) {

	}

	// scalings
	for (int i = 0; i < nodeAni->mNumPositionKeys; ++i) {

	}
}
