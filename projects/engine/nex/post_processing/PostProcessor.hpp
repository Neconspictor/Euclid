#pragma once

namespace nex
{

	class VertexArray;
	class RenderTarget2D;
	class Texture;
	struct Uniform;

	class PostProcessor {
	public:

		/**
		 * Creates a new post processor.
		 * @param width : The screen width
		 * @param height : The screen height
		 */
		PostProcessor(unsigned width, unsigned height);

		// Don't allow inlined destructor for Pimpl 
		~PostProcessor();

		/**
		 * Does post processing. The result is rendered into the given render target.
		 * @param source : The texture to use as a source for the post processing
		 * @param renderTarget : The render target that will be used to store the result of the post processing.
		 */
		void doPostProcessing(Texture* source, RenderTarget2D* output);

		/**
		 * Resizes the post processor for a different resolution.
		 * @param width : The new screen width
		 * @param height : The new screen height
		 */
		void resize(unsigned width, unsigned height);

	private:

		class PostProcessShader;

		void setPostProcessTexture(Texture* texture);

		/**
		 * Used for ping pong rendering.
		 */
		std::unique_ptr<RenderTarget2D> mTemp;
		VertexArray* mFullscreenPlane;

		std::unique_ptr<PostProcessShader> mPostprocessPass;
		unsigned mWidth;
		unsigned mHeight;
	};
}