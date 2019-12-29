#pragma once

#include <functional>
#include <vector>
#include <algorithm>
#include <memory>
#include <nex/renderer/RenderContext.hpp>

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
		virtual void frameUpdate(const RenderContext& constants) = 0;
	};
}