#pragma once
#include <nex/renderer/RenderCommandQueue.hpp>
#include <optional>

namespace nex
{
	class ShaderStorageBuffer;

	/**
	 * A factory class for producing render commands.
	 */
	class RenderCommandFactory {
	public:
		virtual ~RenderCommandFactory() = default;

		/**
		 * Produces a new render commands and adds them to a render command queue.
		 */
		virtual void collectRenderCommands(RenderCommandQueue& queue, bool doCulling, const RenderContext& renderContext) const = 0;
	};
}