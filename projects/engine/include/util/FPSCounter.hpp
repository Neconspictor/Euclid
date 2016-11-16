#ifndef ENGINE_UTIL_FPS_COUNTER_HPP
#define ENGINE_UTIL_FPS_COUNTER_HPP

/**
 * A facility for counting frames per seconds. 
 */
class FPSCounter
{
public:
	FPSCounter()
	{
		counter = 0;
		runtime = 0;
	};
	virtual ~FPSCounter(){};

	long double update(long double timeDiff)
	{
		runtime += timeDiff;
		++counter;
		long double currentFPS = counter / runtime;
		if (!isValid(currentFPS))
		{
			currentFPS = 0;
		}
		if (runtime > 1)
		{
			// restart fps counting
			runtime -= 1;
			counter = 0;
		}

		return currentFPS;
	}

protected:
	static bool isValid(long double value)
	{
		return !isnan(value) && value != HUGE_VALL && value != -HUGE_VALL;
	}

private:
	long double runtime;
	int counter;
};

#endif