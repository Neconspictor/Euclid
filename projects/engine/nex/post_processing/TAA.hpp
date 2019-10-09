#pragma once

#include <memory>
#include <glm/glm.hpp>

namespace nex
{
	class Texture;
	class Camera;

	/**
	 * Implements temporal supersampling anti-aliasing.
	 * 
	 * Terminology:
     *   Feedback:	The feedback specifies the percentage of how much of the current frame is used.
	 *				1 - feedback therefore specifies the percentage of how much of the previous frame is blended into the current frame.
	 *  
	 *  Jitter: Jitter is a slight offset of the projection matrix, so that the scene is slightly rendered differently. This is necessary
	 *          to imitate supersampling. If we use for each frame a different jitter we can capture more pixels. If we blend the jittered results
	 *			the output resembles a supersampled image.
	 */
	class TAA
	{
	public:
		TAA();
		~TAA();

		/**
		 * Advances the jitter and calculates a new jitter matrix.
		 */
		void advanceJitter();

		/**
		 * Renders an antialiased version of the specified source texture into the currently bound render target (color attachment 0).
		 */
		void antialias(Texture* source, Texture* sourceHistory, Texture* depth, const Camera& camera);

		/**
		 * Provides the current feedback value.
		 */
		float getFeedback() const;

		/**
		 * Provides the current jitter matrix.
		 */
		const glm::mat4& getJitterMatrix() const;

		/**
		 * Sets the feedback.
		 */
		void setFeedback(float value);

		/**
		 * updates the jitter vectors so that they match the current render target resolution.
		 * So this function has to be called if the render target resolution has changed.
		 */
		void updateJitterVectors(const glm::vec2& pixelSizeScreenSpace);

		/**
		 * Updates the jitter matrix.
		 */
		void updateJitterMatrix();

	private:

		/**
		 * Advances the jitter cursor.
		 */
		void advanceJitterCursor();

		class TaaPass;

		std::unique_ptr<TaaPass> mTaaPass;


		std::array<glm::vec2, 16> mSampleVector
		{
		  glm::vec2(0.0f, -0.333333333333f),
		  glm::vec2(-0.5f, 0.333333333333f),
		  glm::vec2(0.5f, -0.777777777778f),
		  glm::vec2(-0.75f, -0.111111111111f),
		  glm::vec2(0.25f, 0.555555555556f),
		  glm::vec2(-0.25f, -0.555555555556f),
		  glm::vec2(0.75f, 0.111111111111f),
		  glm::vec2(-0.875f, 0.777777777778f),
		  glm::vec2(0.125f, -0.925925925926f),
		  glm::vec2(-0.375f, -0.259259259259f),
		  glm::vec2(0.625f, 0.407407407407f),
		  glm::vec2(-0.625f, -0.703703703704f),
		  glm::vec2(0.375f, -0.037037037037f),
		  glm::vec2(-0.125f, 0.62962962963f),
		  glm::vec2(0.875f, -0.481481481481f)
		};

		std::array<glm::vec2, 16> mJitterVector;
		glm::mat4 mJitterMatrix;
		unsigned mJitterCursor;
		float mFeedback;
	};
}