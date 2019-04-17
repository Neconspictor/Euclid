#pragma once

namespace nex
{
	class Camera;

	class RenderTarget2D;
	class Texture;
	class Texture2D;
	struct Uniform;
	class DownSampler;
	class GaussianBlur;
	class SMAA;
	class AmbientOcclusionSelector;

	class PostProcessor {
	public:

		/**
		 * Creates a new post processor.
		 * @param width : The screen width
		 * @param height : The screen height
		 * @param downSampler: 
		 * @param gaussianBlur: 
		 */
		PostProcessor(unsigned width, unsigned height, DownSampler* downSampler, GaussianBlur* gaussianBlur);

		// Don't allow inlined destructor for Pimpl 
		~PostProcessor();

		/**
		 * Does post processing. The result is rendered into the given render target.
		 * @param source : The texture to use as a source for the post processing
		 * @param glowTexture : Used for Bloom
		 * @param output : The render target that will be used to store the result of the post processing.
		 */
		nex::Texture2D* doPostProcessing(
			Texture2D* source, 
			Texture2D* glowTexture,
			Texture2D* aoMap,
			RenderTarget2D* output);

		void antialias(Texture2D* source, RenderTarget2D* output);

		SMAA* getSMAA();

		/**
		 * Resizes the post processor for a different resolution.
		 * @param width : The new screen width
		 * @param height : The new screen height
		 */
		void resize(unsigned width, unsigned height);
		AmbientOcclusionSelector* getAOSelector();

	private:

		class PostProcessPass;

		void setAoMap(Texture2D* aoMap);
		void setPostProcessTexture(Texture* texture);
		void setGlowTextures(Texture* halfth, Texture* quarter, Texture* eigth, Texture* sixteenth);

		DownSampler* mDownSampler;
		GaussianBlur* mGaussianBlur;

		std::unique_ptr<PostProcessPass> mPostprocessPass;
		std::unique_ptr<AmbientOcclusionSelector> mAoSelector;

		//Bloom
		std::unique_ptr<RenderTarget2D> mBloomHalfth;
		std::unique_ptr<RenderTarget2D> mBloomQuarter;
		std::unique_ptr<RenderTarget2D> mBloomEigth;
		std::unique_ptr<RenderTarget2D> mBloomSixteenth;
		std::unique_ptr<SMAA> mSmaa;
	};
}