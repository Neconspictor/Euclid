#pragma once

#include <nex/anim/KeyFrame.hpp>
#include <nex/common/File.hpp>
#include <functional>

namespace nex
{
	class KeyFrameAnimationData;

	class KeyFrameAnimation {
	public:

		struct ChannelIDGenerator {
			virtual nex::ChannelID operator()(nex::Sid keyFrameSID) const { return 0; };
		};

		/**
		 * Creates a new unintialized KeyFrameAnimation object.
		 */
		KeyFrameAnimation(const KeyFrameAnimationData& data);

		/**
		 * Creates a new initialized KeyFrameAnimation object.
		 * @param data : Used for initializing the new object.
		 * @param generator: used for generating key frame ids referring to an entity (e.g. bone or vob)
		 */
		KeyFrameAnimation(const KeyFrameAnimationData& data, const ChannelIDGenerator& generator);

		virtual ~KeyFrameAnimation() = default;

		/**
		 * Calculates for a specific animation frame minimum and maximum key frames for position, rotation
		 * and scale for each bone (identified by vector index).
		 * This data can be used to interpolate between keyframes.
		 */
		MixData<int> calcFrameMix(float animationTime) const;

		/**
		 * Calculates transformation matrices from interpolated keyframe data (in bone space).
		 */
		void calcChannelTrafos(float animationTime, std::vector<glm::mat4>& vec) const;

		/**
		 * Provides the name of the animation.
		 */
		const std::string& getName() const;

		/**
		 * Provides the number of ticks of this animation.
		 * Note: tick_count = frame_count - 1
		 */
		float getTickCount() const;

		/**
		 * Provides the total animation key frame count
		 * Note: Return type is a float, but the data will always be lossless convertable to an integer.
		 */
		float getFrameCount()const;

		/**
		 * Provides the ticks per second count.
		 * Note: ticks_per_second = frames_per_second - 1
		 */
		float getTicksPerSecond() const;

		/**
		 * Provides animation duration (in seconds)
		 */
		float getDuration()const;

		virtual void write(nex::BinStream& out) const;
		virtual void load(nex::BinStream& in);


		/**
		 * Creates an unintialized object. Use it at own risk!
		 */
		KeyFrameAnimation() = default;

	protected:
		std::string mName;
		float mTickCount;
		unsigned mChannelCount;
		float mTicksPerSecond;
		std::vector<glm::vec3> mPositions;
		std::vector<glm::quat> mRotations;
		std::vector<glm::vec3> mScales;

		int getNextFrame(const std::vector<bool> flaggedInput, int frameCount, int channelCount, int channelID, int lastFrame);



		template<class Type>
		void createInterpolations(const std::vector<KeyFrame<Type, ChannelID>>& input,
			std::vector<Type>& output,
			int frameCount,
			int channelCount)
		{
			const auto totalCount = frameCount * channelCount;
			output.resize(totalCount);
			std::vector<bool> inputFlags(totalCount, false);

			// first pass: fill raw frames
			for (const auto& key : input) {
				const auto i = key.frame * channelCount + key.id;
				output[i] = key.data;
				inputFlags[i] = true;
			}

			//second pass: interpolate frames

			//Note: each bone has a key frame at frame 0!
			std::vector<ChannelID> lastKeyIndices(channelCount, 0);

			for (int channelID = 0; channelID < channelCount; ++channelID) {

				int frameID = 1;
				while (frameID < frameCount) {
					// search the next suitable keyframe
					const auto lastFrame = frameID - 1;
					auto nextFrame = getNextFrame(inputFlags, frameCount, channelCount, channelID, lastFrame);

					const auto lastIndex = lastFrame * channelCount + channelID;
					const auto& lastData = output[lastIndex];

					auto nextIndex = nextFrame * channelCount + channelID;
					const auto& nextData = output[nextIndex];

					auto interpolationRange = nextFrame - lastFrame;

					if (interpolationRange == 0) {
						nextFrame = frameCount - 1;
						nextIndex = nextFrame * channelCount + channelID;
						interpolationRange = nextFrame - lastFrame;
						output[nextIndex] = nextData;
					}

					// start at index 1, since we don't want to interpolate lastIndex (unncessary)
					for (int i = 1; i < interpolationRange; ++i) {
						const auto ratio = i / static_cast<float>(interpolationRange);
						output[lastIndex + i * channelCount] = KeyFrame<Type, ChannelID>::mix(lastData, nextData, ratio);
					}

					// Note: +1 is important, since (next) lastFrame has to be set to (current) nextFrame
					frameID = nextFrame + 1;
				}
			}
		}

		void init(const KeyFrameAnimationData& data, const ChannelIDGenerator& generator);
	};

	class KeyFrameAnimationData
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

		unsigned getChannelCount() const;

		/**
		 * Provides the number of frames.
		 * Note: Return type is a float, but the data will always be lossless convertable to an integer.
		 */
		float getTickCount()const;

		/**
		 * Sets the total number of channels used in the animation.
		 */
		void setChannelCount(unsigned channelCount);

		/**
		 * Sets the name of the animation.
		 */
		void setName(const std::string& name);

		/**
		 * Sets the tick count of the animation.
		 */
		void setTickCount(float tickCount);

		/**
		 * Sets the number of ticks (frame changes)  for this animation.
		 */
		void setTicksPerSecond(float ticksPerSeconds);

		unsigned mChannelCount;
		std::string mName;
		float mTickCount;
		float mTicksPerSecond;

		std::vector<KeyFrame<glm::vec3, Sid>> mPositionKeys;
		std::vector<KeyFrame<glm::quat, Sid>> mRotationKeys;
		std::vector<KeyFrame<glm::vec3, Sid>> mScaleKeys;
	};


	nex::BinStream& operator>>(nex::BinStream& in, KeyFrameAnimation& ani);
	nex::BinStream& operator<<(nex::BinStream& out, const KeyFrameAnimation& ani);
}