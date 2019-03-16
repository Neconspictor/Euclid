#pragma once
#include <memory>

namespace nex {
	class RenderTarget2D;
	class Sampler;
	class Texture2D;

	class DownSampler
	{
	public:

		DownSampler(unsigned width, unsigned height);

		~DownSampler();

		Texture2D* downsampleHalfResolution(Texture2D* src);

		Texture2D* downsampleQuarterResolution(Texture2D* src);

		Texture2D* downsampleEigthResolution(Texture2D* src);

		Texture2D* downsampleSixteenthResolution(Texture2D* src);

		/**
		 * Renders a texture to the
		 */
		nex::Texture2D* downsample(Texture2D* src, RenderTarget2D* dest);

		void resize(unsigned width, unsigned height);

	private:

		class DownSampleShader;
		std::unique_ptr<DownSampleShader> mDownSampleShader;
		std::unique_ptr<RenderTarget2D> mHalfResolution;
		std::unique_ptr<RenderTarget2D> mQuarterResolution;
		std::unique_ptr<RenderTarget2D> mEigthResolution;
		std::unique_ptr<RenderTarget2D> mSixteenthResolution;
		std::unique_ptr<Sampler> mSampler;
	};
}
