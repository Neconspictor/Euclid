#pragma once

namespace nex
{
	class Camera;

	class RenderTarget;
	class RenderTarget2D;
	class Texture;
	class Texture2D;
	struct Uniform;
	class DownSampler;
	class GaussianBlur;
	class SMAA;
	class AmbientOcclusionSelector;
	class FXAA;
	class TAA;
	class SSR;

	class PostProcessor {
	public:

		struct BloomTextures 
		{
			Texture* bloomHalf;
			Texture* bloomQuarter;
			Texture* bloomEighth;
			Texture* bloomSixteenth;
		};

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


		BloomTextures computeBloom(Texture2D* source,
			Texture2D* bloomTexture);

		/**
		 * Does post processing. The result is rendered into the given render target.
		 * @param source : The texture to use as a source for the post processing
		 * @param glowTexture : Used for Bloom
		 * @param output : The render target that will be used to store the result of the post processing.
		 */
		void doPostProcessing(
			Texture2D* source, 
			Texture2D* glowTexture,
			Texture2D* aoMap,
			Texture2D* motionMap,
			const BloomTextures& bloomTextures);

		void antialias(Texture2D* source, RenderTarget* output);

		SMAA* getSMAA();

		FXAA* getFXAA();

		TAA* getTAA();

		SSR* getSSR();

		/**
		 * Resizes the post processor for a different resolution.
		 * @param width : The new screen width
		 * @param height : The new screen height
		 */
		void resize(unsigned width, unsigned height);
		AmbientOcclusionSelector* getAOSelector();

		void renderAO(Texture* aoMap);

	private:

		class PostProcessPass;
		class AoPass;

		void setAoMap(Texture2D* aoMap);
		void setMotionMap(Texture2D* motionMap);
		void setPostProcessTexture(Texture* texture);
		void setGlowTextures(Texture* halfth, Texture* quarter, Texture* eigth, Texture* sixteenth);

		DownSampler* mDownSampler;
		GaussianBlur* mGaussianBlur;

		std::unique_ptr<PostProcessPass> mPostprocessPass;
		std::unique_ptr<AoPass> mAoPass;
		std::unique_ptr<AmbientOcclusionSelector> mAoSelector;

		//Bloom
		std::unique_ptr<RenderTarget2D> mBloomHalfth;
		std::unique_ptr<RenderTarget2D> mBloomQuarter;
		std::unique_ptr<RenderTarget2D> mBloomEighth;
		std::unique_ptr<RenderTarget2D> mBloomSixteenth;

		std::unique_ptr<SMAA> mSmaa;
		std::unique_ptr<FXAA> mFxaa;
		std::unique_ptr<TAA> mTaa;

		std::unique_ptr<SSR> mSSR;
	};
}