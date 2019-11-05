#pragma once

#include <nex/anim/KeyFrame.hpp>

namespace nex
{
	class Rig;

	class BoneAnimation
	{
	public:

		/**
		 * Adds a position key frame.
		 */
		void addPositionKey(PositionKeyFrame keyFrame);

		/**
		 * Adds a position key frame.
		 */
		void addRotationKey(RotationKeyFrame keyFrame);

		/**
		 * Adds a position key frame.
		 */
		void addScaleKey(ScaleKeyFrame keyFrame);

		const std::string& getName() const;
		void setName(const std::string& name);

		/**
		 * Provides the optimized position key frames.
		 * NOTE: optimize() function has to be called prior!
		 */
		const std::vector<OptPositionKeyFrame>& getOptPositionKeys() const;

		/**
		 * Provides the optimized rotation key frames.
		 * NOTE: optimize() function has to be called prior!
		 */
		const std::vector<OptRotationKeyFrame>& getOptRotationKeys() const;

		/**
		 * Provides the optimized scale key frames.
		 * NOTE: optimize() function has to be called prior!
		 */
		const std::vector<OptScaleKeyFrame>& getOptScaleKeys() const;

		/**
		 * Provides the rig this bone animation belongs to.
		 */
		const Rig* getRig() const;
		Rig* getRig();

		/**
		 * Provides the total animation key frame count (ticks)
		 */
		double getTicks()const;

		/**
		 * Sets the totoal animation key frame count (ticks).
		 */
		void setTicks(double ticks);

		/**
		 * Provides the amount of ticks that should be played per second.
		 */
		double getTicksPerSecond()const;

		/**
		 * Sets the tick count that should be played per second.
		 */
		void setTicksPerSecond(double ticksPerSecond);

		/**
		 * Provides animation duration (in seconds)
		 */
		double getDuration()const;

		/**
		 * Optimizes internal structures and connects key frames to bones.
		 * Sets the rig for this bone animation.
		 * @throws std::invalid_argument :  - if rig is nullptr
		 *									- if this function was called once before.
		 */
		void optimize(Rig* rig);

	private:
		std::string mName;
		double mTicks;
		double mTicksPerSecond;
		Rig* mRig = nullptr;

		std::set<PositionKeyFrame, nex::KeyFrame::Comparator> mPositionKeys;
		std::set<RotationKeyFrame, nex::KeyFrame::Comparator> mRotationKeys;
		std::set<ScaleKeyFrame, nex::KeyFrame::Comparator> mScaleKeys;

		std::vector<OptPositionKeyFrame> mPositionsOpt;
		std::vector<OptRotationKeyFrame> mRotationsOpt;
		std::vector<OptScaleKeyFrame> mScalesOpt;

		bool mOptimized;
	};
}