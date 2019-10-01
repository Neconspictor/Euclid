#pragma once

namespace nex
{
	class RenderTarget;
	class RenderTarget2D;
	class Texture2D;
	class Sampler;

	class SMAA
	{
	public:
		SMAA(unsigned width, unsigned height);
		~SMAA();

		Texture2D* getArexTex();
		Texture2D* getSearchTex();


		Texture2D* getEdgeDetection();
		Texture2D* getBlendingWeight();

		void resize(unsigned width, unsigned height);

		Texture2D* renderEdgeDetectionPass(Texture2D* colorTexGamma);

		Texture2D* renderBlendingWeigthCalculationPass(Texture2D* edgeTex);

		void renderNeighborhoodBlendingPass(Texture2D* blendTex, Texture2D* colorTex, RenderTarget* output);

		void reset();

	private:

		class EdgeDetectionPass;
		class BlendingWeightCalculationPass;
		class NeighborhoodBlendingPass;

		std::unique_ptr<RenderTarget2D> mEdgesTex;
		std::unique_ptr<RenderTarget2D> mBlendTex;

		std::unique_ptr<Texture2D> mAreaTex;
		std::unique_ptr<Texture2D> mSearchTex;
		std::unique_ptr<Sampler> mBilinearFilter;
		std::unique_ptr<Sampler> mPointFilter;

		std::unique_ptr<EdgeDetectionPass> mEdgeDetectionShader;
		std::unique_ptr<BlendingWeightCalculationPass> mBlendingWeightCalculationShader;
		std::unique_ptr<NeighborhoodBlendingPass> mNeighborhoodBlendingShader;
	};
}