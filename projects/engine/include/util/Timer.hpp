#ifndef ENGINE_UTIL_TIMER
#define ENGINE_UTIL_TIMER
#include <chrono>

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
	virtual ~Timer(){};

	/**
	 * Returns the time difference since the last update function call
	 * measured in seconds.
	 * E.g. if the last call of this function happend 1/2 second ago, this function
	 * will return 0.5 .
	 */
	long double update()
	{
		using namespace std::chrono;
		auto currentFrameTime = timer.now();
		if (!lastUpdateTime.time_since_epoch().count())
		{
			lastUpdateTime = currentFrameTime;
		}

		auto diff = duration_cast<duration<long double>>(currentFrameTime - lastUpdateTime);
		lastUpdateDiff = diff.count();
		lastUpdateTime = currentFrameTime;
		return lastUpdateDiff;
	};

	/**
	 * Provides the time difference calculated by the last function call of Timer::update().
	 */
	long double getLastUpdateTimeDifference() const
	{
		return lastUpdateDiff;
	}

private:
	std::chrono::high_resolution_clock timer;
	std::chrono::time_point<std::chrono::steady_clock> lastUpdateTime;
	long double lastUpdateDiff;
};

#endif