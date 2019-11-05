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
		std::vector<BoneAnimation> load(const aiScene* scene, Rig* rig);

	protected:
		void loadBoneChannel(BoneAnimation& boneAni, aiNodeAnim* nodeAni);
	};
}