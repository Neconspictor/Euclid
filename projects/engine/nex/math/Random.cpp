#include <nex/math/Random.hpp>

nex::UniformRandom::UniformRandom(double minValue, double maxValue) : mGenerator(std::random_device()()), mDistribution(minValue, maxValue)
{
}

double nex::UniformRandom::next()
{
	return mDistribution(mGenerator);
}

double nex::Random::nextDouble()
{
	static UniformRandom random(0.0, 1.0);
	return random.next();
}

float nex::Random::nextFloat()
{
	return static_cast<float>(nextDouble());
}