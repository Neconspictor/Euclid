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
			lastUpdateDiff = 0;
		};
		virtual ~Timer() {};

		/**
		 * Returns the time difference since the last update function call
		 * measured in seconds.
		 * E.g. if the last call of this function happend 1/2 second ago, this function
		 * will return 0.5 .
		 */
		float update()
		{
			using namespace std::chrono;
			auto currentFrameTime = timer.now();
			if (!lastUpdateTime.time_since_epoch().count())
			{
				lastUpdateTime = currentFrameTime;
			}

			auto diff = duration_cast<duration<float>>(currentFrameTime - lastUpdateTime);
			lastUpdateDiff = diff.count();
			lastUpdateTime = currentFrameTime;
			return lastUpdateDiff;
		};

		/**
		 * Provides the time difference calculated by the last function call of Timer::update().
		 */
		float getLastUpdateTimeDifference() const
		{
			return lastUpdateDiff;
		}

	private:
		std::chrono::high_resolution_clock timer;
		std::chrono::high_resolution_clock::time_point lastUpdateTime;
		float lastUpdateDiff;
	};
}