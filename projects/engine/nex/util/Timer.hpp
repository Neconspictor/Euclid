#pragma once
#include <chrono>

namespace nex
{

	/**
	 * A timer mesaures the time past between two time points.
	 */
	class Timer
	{
	public:
		Timer()
		{			
			mCurrentTimePoint = nanos();
			mDiff = 0;
			mCountedTime = 0;
			mPaused = false;
		};

		void reset() {
			mCurrentTimePoint = nanos();
			mDiff = 0;
		}

		void update()
		{
			if (isPaused()) return;
			using namespace std::chrono;
			const auto currentFrameTime = nanos();
			mDiff = currentFrameTime - mCurrentTimePoint;
			mCurrentTimePoint = currentFrameTime;
			mCountedTime += mDiff;
		};

		float getCountedTimeInSeconds() {
			return mCountedTime / (long double)1000000000;
		}

		float getTimeDiffInSeconds()
		{
			return mDiff / (long double)1000000000;
		}

		bool isPaused() const {
			return mPaused;
		}

		void pause(bool pauseTimer) {

			if (mPaused == pauseTimer) return;

			if (pauseTimer) {
				mPausedDiff = nanos() - mCurrentTimePoint;
			}
			else {
				mCurrentTimePoint = nanos() - mPausedDiff;
			}

			mPaused = pauseTimer;
		}


	private:
		uint64_t mCurrentTimePoint;
		uint64_t mDiff;
		uint64_t mCountedTime;
		uint64_t mPausedDiff;
		bool mPaused;

		// Get time stamp in nanoseconds.
		uint64_t nanos()
		{
			uint64_t ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::
				now().time_since_epoch()).count();
			return ns;
		}
	};


	class SimpleTimer
	{
	public:
		float mCurrentTimePoint = 0;
		float mDiff = 0;

		void reset(float time)
		{
			mCurrentTimePoint = time;
			mDiff = 0;
		}

		void update(float time)
		{
			mDiff = time - mCurrentTimePoint;
			mCurrentTimePoint = time;

		}
	};
}