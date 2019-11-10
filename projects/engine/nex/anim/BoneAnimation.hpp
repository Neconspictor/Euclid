#pragma once

#include <nex/anim/KeyFrame.hpp>

namespace nex
{
	class Rig;
	class BoneAnimationData;

	class BoneAnimation {

	public:
		using BoneID = short;
		
		BoneAnimation(const BoneAnimationData& data);


		/**
		 * Calculates for a specific animation time minimum and maximum key frames for position, rotation
		 * and scale for each bone (identified by vector index). 
		 * This data can be used to interpolate between keyframes.
		 */
		std::vector<MinMaxKeyFrame> calcMinMaxKeyFrames(float animationTime) const;

		/**
		 * Calculates interpolated transformation data.
		 */
		static std::vector<CompoundKeyFrame> calcInterpolatedTrafo(const std::vector<MinMaxKeyFrame>& minMaxs, float animationTime);

		/**
		 * Calculates bone transformation matrices from interpolated keyframe data (in bone space).
		 */
		static std::vector<glm::mat4> calcBoneTrafo(const std::vector<CompoundKeyFrame>& data);

		/**
		 * Propagates matrix transformations from parent bone to children bones.
		 * Note: input and output is expected to be in bone space.
		 */
		void applyParentHierarchyTrafos(std::vector<glm::mat4>& vec) const;

		/**
		 * Note: Should be called after applyParentHierarchyTrafos
		 */
		void applyLocalToBoneSpaceTrafos(std::vector<glm::mat4>& vec) const;


		/**
		 * Provides the name of the animation.
		 */
		const std::string& getName() const;

		/**
		 * Provides the rig this bone animation belongs to.
		 */
		const Rig* getRig() const;

		/**
		 * Provides the total animation key frame count (ticks)
		 */
		float getTicks()const;

		/**
		 * Provides the amount of ticks that should be played per second.
		 */
		float getTicksPerSecond() const;

		/**
		 * Provides animation duration (in seconds)
		 */
		float getDuration()const;

	private:

		void calcMinMaxKeyId(std::vector<MinMaxKeyFrame>& keys, 
			size_t structOffset,
			const float animationTime,
			unsigned& boneID,
			const float currentTime,
			const unsigned currentKeyID,
			const unsigned currentBoneID) const;

		std::string mName;
		float mTicks;
		float mTicksPerSecond;
		const Rig* mRig = nullptr;
		std::vector<PositionKeyFrame<BoneID>> mPositions;
		std::vector<RotationKeyFrame<BoneID>> mRotations;
		std::vector<ScaleKeyFrame<BoneID>> mScales;
	};

	class BoneAnimationData
	{
	public:

		using SID = unsigned;

		/**
		 * Adds a position key frame.
		 */
		void addPositionKey(PositionKeyFrame<SID> keyFrame);

		/**
		 * Adds a position key frame.
		 */
		void addRotationKey(RotationKeyFrame<SID> keyFrame);

		/**
		 * Adds a position key frame.
		 */
		void addScaleKey(ScaleKeyFrame<SID> keyFrame);

		/**
		 * Sets the name of the animation.
		 */
		void setName(const std::string& name);

		/**
		 * Sets the rig this animation references to.
		 */
		void setRig(const Rig* rig);

		/**
		 * Sets the totoal animation key frame count (ticks).
		 */
		void setTicks(float ticks);

		/**
		 * Sets the tick count that should be played per second.
		 */
		void setTicksPerSecond(float ticksPerSecond);

	private:

		friend BoneAnimation;

		std::string mName;
		float mTicks;
		float mTicksPerSecond;
		const Rig* mRig = nullptr;

		std::vector<PositionKeyFrame<SID>> mPositionKeys;
		std::vector<RotationKeyFrame<SID>> mRotationKeys;
		std::vector<ScaleKeyFrame<SID>> mScaleKeys;
	};
}