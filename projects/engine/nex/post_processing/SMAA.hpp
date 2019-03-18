#pragma once

namespace nex
{
	class RenderTarget2D;
	class Texture2D;
	class Sampler;
	class VertexArray;

	class SMAA
	{
	public:
		SMAA(unsigned width, unsigned height);
		~SMAA();

		Texture2D* getArexTex();
		Texture2D* getSearchTex();


		void resize(unsigned width, unsigned height);

		Texture2D* renderEdgeDetectionPass(Texture2D* colorTexGamma);

		void reset();

	private:

		class EdgeDetectionShader;

		std::unique_ptr<RenderTarget2D> mEdgesTex;
		std::unique_ptr<RenderTarget2D> mBlendTex;

		std::unique_ptr<Texture2D> mAreaTex;
		std::unique_ptr<Texture2D> mSearchTex;
		std::unique_ptr<Sampler> mBilinearFilter;
		std::unique_ptr<Sampler> mPointFilter;

		std::unique_ptr<EdgeDetectionShader> mEdgeDetectionShader;

		VertexArray* mFullscreenTriangle;
	};
}