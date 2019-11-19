#pragma once

#include <nex/anim/KeyFrame.hpp>
#include <nex/common/File.hpp>

namespace nex
{
	class Rig;
	class BoneAnimationData;

	class BoneAnimation {

	public:		
		BoneAnimation(const BoneAnimationData& data);


		/**
		 * Calculates for a specific animation frame minimum and maximum key frames for position, rotation
		 * and scale for each bone (identified by vector index). 
		 * This data can be used to interpolate between keyframes.
		 */
		MixData<int> calcFrameMix(float animationTime) const;

		/**
		 * Calculates bone transformation matrices from interpolated keyframe data (in bone space).
		 */
		void calcBoneTrafo(float animationTime, std::vector<glm::mat4>& vec) const;

		/**
		 * Propagates matrix transformations from parent bone to children bones.
		 * Note: input and output is expected to be in bone space.
		 */
		void applyParentHierarchyTrafos(std::vector<glm::mat4>& vec) const;


		/**
		 * Provides the name of the animation.
		 */
		const std::string& getName() const;

		/**
		 * Provides the rig this bone animation belongs to.
		 */
		const Rig* getRig() const;

		/**
		 * Provides the id of the rig this bone animation belongs to.
		 */
		const std::string& getRigID() const;

		/**
		 * Provides the sid of the rig this bone animation belongs to.
		 */
		unsigned getRigSID() const;

		/**
		 * Provides the total animation key frame count (ticks)
		 * Note: Return type is a float, but the data will always be lossless convertable to an integer.
		 */
		float getFrameCount()const;

		/**
		 * Provides the amount of frames that should be played per second.
		 */
		float getFramesPerSecond() const;

		/**
		 * Provides animation duration (in seconds)
		 */
		float getDuration()const;

		static void write(nex::BinStream&  out, const BoneAnimation& ani);
		static void load(nex::BinStream& in, BoneAnimation& ani);

		/**
		 * Creates an unintialized animation, that is not usable.
		 * NOTE: Use at own risk!
		 */
		static BoneAnimation createUnintialized();

	private:

		BoneAnimation() = default;

		template<class Type>
		void createInterpolations(const std::vector<KeyFrame<Type, BoneID>>& input,
			std::vector<Type>& output,
			int frameCount,
			int boneCount) 
		{
			const auto totalCount = frameCount * boneCount;
			output.resize(totalCount);
			std::vector<bool> inputFlags(totalCount, false);

			// first pass: fill raw frames
			for (const auto& key : input) {
				const auto i = key.frame * boneCount + key.id;
				output[i] = key.data;
				inputFlags[i] = true;
			}

			//second pass: interpolate frames

			//Note: each bone has a key frame at frame 0!
			std::vector<BoneID> lastKeyIndices(boneCount, 0);

			for (int boneID = 0; boneID < boneCount; ++boneID) {

				int frameID = 1;
				while (frameID < frameCount) {
					// search the next suitable keyframe
					const auto lastFrame = frameID - 1;
					auto nextFrame = getNextFrame(inputFlags, frameCount, boneCount, boneID, lastFrame);

					const auto lastIndex = lastFrame * boneCount + boneID;
					const auto& lastData = output[lastIndex];

					auto nextIndex = nextFrame * boneCount + boneID;
					const auto& nextData = output[nextIndex];

					auto interpolationRange = nextFrame - lastFrame;

					if (interpolationRange == 0) {
						nextFrame = frameCount - 1;
						nextIndex = nextFrame * boneCount + boneID;
						interpolationRange = nextFrame - lastFrame;
						output[nextIndex] = nextData;
					}

					// start at index 1, since we don't want to interpolate lastIndex (unncessary)
					for (int i = 1; i < interpolationRange; ++i) {
						const auto ratio = i / static_cast<float>(interpolationRange);
						output[lastIndex + i * boneCount] = KeyFrame<Type, BoneID>::mix(lastData, nextData, ratio);
					}

					// Note: +1 is important, since (next) lastFrame has to be set to (current) nextFrame
					frameID = nextFrame + 1;
				}
			}
		}



		int getNextFrame(const std::vector<bool> flaggedInput, int frameCount, int boneCount, int boneID, int lastFrame);

		std::string mName;
		float mFrameCount;
		unsigned mBoneCount;
		float mFramesPerSecond;
		std::string mRigID;
		unsigned mRigSID;
		std::vector<glm::vec3> mPositions;
		std::vector<glm::quat> mRotations;
		std::vector<glm::vec3> mScales;
	};

	nex::BinStream& operator>>(nex::BinStream& in, BoneAnimation& ani);
	nex::BinStream& operator<<(nex::BinStream& out, const BoneAnimation& ani);

	class BoneAnimationData
	{
	public:

		using Sid = unsigned;

		/**
		 * Adds a position key frame.
		 */
		void addPositionKey(KeyFrame<glm::vec3, Sid> keyFrame);

		/**
		 * Adds a position key frame.
		 */
		void addRotationKey(KeyFrame<glm::quat, Sid> keyFrame);

		/**
		 * Adds a position key frame.
		 */
		void addScaleKey(KeyFrame<glm::vec3, Sid> keyFrame);

		/**
		 * Provides the number of frames.
		 * Note: Return type is a float, but the data will always be lossless convertable to an integer.
		 */
		float getFrameCount()const;

		/**
		 * Sets the name of the animation.
		 */
		void setName(const std::string& name);

		/**
		 * Sets the rig this animation references to.
		 */
		void setRig(const Rig* rig);

		/**
		 * Sets the totoal animation key frame count.
		 */
		void setFrameCount(float frameCount);

		/**
		 * Sets the frae count that should be played per second.
		 */
		void setFramesPerSecond(float framesPerSecond);

	private:

		friend BoneAnimation;

		std::string mName;
		float mFrameCount;
		float mFramesPerSecond;
		const Rig* mRig = nullptr;

		std::vector<KeyFrame<glm::vec3, Sid>> mPositionKeys;
		std::vector<KeyFrame<glm::quat, Sid>> mRotationKeys;
		std::vector<KeyFrame<glm::vec3, Sid>> mScaleKeys;
	};
}