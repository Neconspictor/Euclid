#pragma once

namespace nex
{
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
		virtual ~FPSCounter() {};

		Real update(Real timeDiff)
		{
			runtime += timeDiff;
			++counter;
			Real currentFPS = counter / runtime;
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
		static bool isValid(Real value)
		{
			return !isnan(value) && value != HUGE_VALL && value != -HUGE_VALL;
		}

	private:
		Real runtime;
		int counter;
	};
}