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
			
			lastUpdateTime = nanos();
			diff = 0;
		};
		~Timer() {};

		void update()
		{
			using namespace std::chrono;
			const auto currentFrameTime = nanos();
			diff = currentFrameTime - lastUpdateTime;
			lastUpdateTime = currentFrameTime;
		};

		float getTimeInSeconds()
		{
			return diff / (long double)1000000000;
		}

		uint64_t getTimeInNanos()
		{
			return diff;
		}

		uint64_t getTimeInMicros()
		{
			return diff / (long double)1000;
		}

		uint64_t getTimeInMillis()
		{
			return diff / (long double)1000000;
		}


	private:
		uint64_t lastUpdateTime;
		uint64_t diff;



		// Get time stamp in milliseconds.
		uint64_t millis()
		{
			uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::
				now().time_since_epoch()).count();
			return ms;
		}

		// Get time stamp in microseconds.
		uint64_t micros()
		{
			uint64_t us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::
				now().time_since_epoch()).count();
			return us;
		}

		// Get time stamp in nanoseconds.
		uint64_t nanos()
		{
			uint64_t ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::
				now().time_since_epoch()).count();
			return ns;
		}

		// Get time stamp in seconds.
		uint64_t seconds()
		{
			uint64_t ns = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::
				now().time_since_epoch()).count();
			return ns;
		}

	};
}