#include <nex/anim/BoneAnimation.hpp>
#include "BoneAnimation.hpp"

const std::string& nex::BoneAnimation::getName() const
{
	return mName;
}

void nex::BoneAnimation::setName(const std::string& name)
{
	mName = name;
}

double nex::BoneAnimation::getTicks() const
{
	return mTicks;
}

void nex::BoneAnimation::setTicks(double ticks)
{
	mTicks = ticks;
}

double nex::BoneAnimation::getTicksPerSecond() const
{
	return mTicksPerSecond;
}

void nex::BoneAnimation::setTicksPerSecond(double ticksPerSecond)
{
	mTicksPerSecond = ticksPerSecond;
}

double nex::BoneAnimation::getDuration() const
{
	return mTicks / mTicksPerSecond;
}