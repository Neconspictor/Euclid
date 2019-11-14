#pragma once

#include <functional>
#include <vector>
#include <algorithm>
#include <memory>

namespace nex
{
	/**
	 * Interface for classes needed to be updated every frame.
	 */
	class FrameUpdateable {
	public:
		virtual ~FrameUpdateable() = default;

		/**
		 * Updates the object for the current frame.
		 */
		virtual void frameUpdate(float frameTime) = 0;
	};
}