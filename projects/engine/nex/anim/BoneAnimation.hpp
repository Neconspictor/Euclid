#pragma once

#include <nex/anim/KeyFrame.hpp>

namespace nex
{
	class Rig;
	class BoneAnimation;

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

		std::set<PositionKeyFrame<SID>, nex::KeyFrame<SID>::Comparator> mPositionKeys;
		std::set<RotationKeyFrame<SID>, nex::KeyFrame<SID>::Comparator> mRotationKeys;
		std::set<ScaleKeyFrame<SID>, nex::KeyFrame<SID>::Comparator> mScaleKeys;
	};

	class BoneAnimation {

	public:
		using BoneID = short;
		
		BoneAnimation(const BoneAnimationData& data);

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

		std::string mName;
		float mTicks;
		float mTicksPerSecond;
		const Rig* mRig = nullptr;
		std::vector<PositionKeyFrame<BoneID>> mPositions;
		std::vector<RotationKeyFrame<BoneID>> mRotations;
		std::vector<RotationKeyFrame<BoneID>> mScales;
	};
}