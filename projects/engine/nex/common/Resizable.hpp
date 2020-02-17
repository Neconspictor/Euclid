#pragma once

#include <functional>
#include <vector>
#include <algorithm>
#include <memory>
#include <nex/renderer/RenderContext.hpp>

namespace nex
{
	/**
	 * Interface for classes needed to be updated when window size changes.
	 */
	class Resizable {
	public:
		virtual ~Resizable() = default;

		/**
		 * Resizes the object when the window dimension changes.
		 */
		virtual void resize(unsigned width, unsigned height) = 0;
	};
}