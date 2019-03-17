#pragma once

namespace nex
{
	class RenderTarget2D;
	class Texture2D;
	class Sampler;

	class SMAA
	{
	public:
		SMAA(unsigned width, unsigned height);
		~SMAA();


		void resize(unsigned width, unsigned height);

		void reset();

	private:
		std::unique_ptr<RenderTarget2D> mEdgesTex;
		std::unique_ptr<RenderTarget2D> mBlendTex;

		std::unique_ptr<Texture2D> mAreaTex;
		std::unique_ptr<Texture2D> mSearchTex;
		std::unique_ptr<Sampler> mBilinearFilter;
		std::unique_ptr<Sampler> mPointFilter;
	};
}