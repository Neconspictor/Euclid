#pragma once
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

	float update(float timeDiff)
	{
		runtime += timeDiff;
		++counter;
		float currentFPS = counter / runtime;
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
	static bool isValid(float value)
	{
		return !isnan(value) && value != HUGE_VALL && value != -HUGE_VALL;
	}

private:
	float runtime;
	int counter;
};