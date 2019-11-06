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

		BoneAnimation boneAni;
		boneAni.setName(ani->mName.C_Str());
		boneAni.setTicks(ani->mDuration);
		boneAni.setTicksPerSecond(ani->mTicksPerSecond);

		for (auto j = 0; j < ani->mNumChannels; ++j) {
			loadBoneChannel(boneAni, ani->mChannels[j]);
		}

		boneAni.optimize(rig);

		anims.emplace_back(std::move(boneAni));
	}

	return anims;
}

void nex::BoneAnimationLoader::loadBoneChannel(BoneAnimation& boneAni, aiNodeAnim* nodeAni)
{
	auto nodeName = nodeAni->mNodeName;
	auto sid = SID(nodeName.C_Str());
	
	//positions
	for (int i = 0; i < nodeAni->mNumPositionKeys; ++i) {
		const auto& key = nodeAni->mPositionKeys[i];
		const auto& p = key.mValue;
		boneAni.addPositionKey({ sid, key.mTime, {p.x, p.y, p.z} });
	}

	// rotations
	for (int i = 0; i < nodeAni->mNumRotationKeys; ++i) {
		const auto& key = nodeAni->mRotationKeys[i];
		const auto& q = key.mValue;
		boneAni.addRotationKey({ sid, key.mTime, {q.x, q.y, q.z, q.w} });
	}

	// scalings
	for (int i = 0; i < nodeAni->mNumScalingKeys; ++i) {
		const auto& key = nodeAni->mScalingKeys[i];
		const auto& s = key.mValue;
		boneAni.addScaleKey({ sid, key.mTime, {s.x, s.y, s.z} });
	}
}