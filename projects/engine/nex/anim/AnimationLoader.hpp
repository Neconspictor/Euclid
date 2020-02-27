#pragma once

#include <vector>
#include <nex/anim/BoneAnimation.hpp>
#include <assimp/scene.h>

namespace nex
{
	class Rig;

	class KeyFrameAnimationLoader
	{
	public:
		virtual ~KeyFrameAnimationLoader() = default;

		/**
		 * Loads a keyframe animation from an aiAnimation object.
		 * @param ani : The aiAnimation object to extract the keyframe animation from.
		 * @param aniName : The name for the keysframe animation. Note: Can be arbitrary choosen by user.
		 * @param generator : A channel id generator used for creating ids for the keyframes. This ids can be used to link a keyframe to a vob or bone etc. . 
		 */
		nex::KeyFrameAnimation load(const aiAnimation* ani, const std::string& aniName, const KeyFrameAnimation::ChannelIDGenerator& generator);
	protected:
		void fillChannel(KeyFrameAnimationData& data, aiNodeAnim* nodeAni);
		void fillData(nex::KeyFrameAnimationData& output, const aiAnimation* ani, const std::string& aniName);
	};

	class BoneAnimationLoader : public KeyFrameAnimationLoader
	{
	public:
		virtual ~BoneAnimationLoader() = default;

		/**
		 * Loads a bone animation from an aiAnimation object.
		 * @param rig : The rig the resulting bone animation is designed for.
		 * @param aniName : The name the bone animation should have.
		 */
		nex::BoneAnimation load(const aiAnimation* ani, const Rig* rig, const std::string& aniName);
	};
}