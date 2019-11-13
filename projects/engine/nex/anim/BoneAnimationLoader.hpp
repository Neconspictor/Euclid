#pragma once

#include <vector>
#include <nex/anim/BoneAnimation.hpp>
#include <assimp/scene.h>

namespace nex
{
	class Rig;

	class BoneAnimationLoader
	{
	public:
		std::unique_ptr<nex::BoneAnimation> load(const aiScene* scene, const Rig* rig, const std::string& aniName);

	protected:
		void loadBoneChannel(BoneAnimationData& boneAni, aiNodeAnim* nodeAni);
	};
}