#pragma once

#include <nex/texture/RenderTarget.hpp>
#include <nex/shader/Shader.hpp>

namespace nex
{

	class VertexArray;

	class PostProcessor {
	public:

		/**
		 * Creates a new post processor.
		 * @param width : The screen width
		 * @param height : The screen height
		 */
		PostProcessor(unsigned width, unsigned height);

		/**
		 * Does post processing. The result is rendered into the given render target.
		 * @param renderTarget : The render target to use for post processing.
		 */
		void doPostProcessing(RenderTarget2D* renderTarget);

		/**
		 * Resizes the post processor for a different resolution.
		 * @param width : The new screen width
		 * @param height : The new screen height
		 */
		void resize(unsigned width, unsigned height);

	private:

		/**
		 * Used for ping pong rendering.
		 */
		std::unique_ptr<RenderTarget2D> mTemp;
		VertexArray* mFullscreenPlane;

		std::unique_ptr<Shader> mPostprocessPass;
	};
}