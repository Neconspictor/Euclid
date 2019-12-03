#pragma once

#include <random>

namespace nex {

	enum class Distribution {
		UNIFORM
	};

	class Random {
	public:
		static double nextDouble();
		static float nextFloat();
	};

	class UniformRandom {
	public:

		UniformRandom(double minValue, double maxValue);

		double next();
	
	private:
		std::mt19937 mGenerator;
		std::uniform_real_distribution<> mDistribution;
	};

}