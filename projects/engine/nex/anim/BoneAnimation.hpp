#pragma once

namespace nex
{
	class BoneAnimation
	{
	public:

		BoneAnimation();

		const std::string& getName() const;
		void setName(const std::string& name);

		/**
		 * Provides the total animation key frame count (ticks)
		 */
		double getTicks()const;

		/**
		 * Sets the totoal animation key frame count (ticks).
		 */
		void setTicks(double ticks);

		/**
		 * Provides the amount of ticks that should be played per second.
		 */
		double getTicksPerSecond()const;

		/**
		 * Sets the tick count that should be played per second.
		 */
		void setTicksPerSecond(double ticksPerSecond);

		/**
		 * Provides animation duration (in seconds)
		 */
		double getDuration()const;

		void setPositionKey(const std::string& boneName, const glm::vec3& position, double );


	private:
		std::string mName;
		double mTicks;
		double mTicksPerSecond;

	};
}