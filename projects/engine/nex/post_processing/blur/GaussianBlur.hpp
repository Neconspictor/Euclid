#pragma once

namespace nex {
	class RenderTarget2D;
	class Sampler;
	class Texture2D;
	class GaussianBlurHorizontalShader;
	class GaussianBlurVerticalShader;

	class GaussianBlur {

	public:
		GaussianBlur(unsigned width, unsigned height);

		~GaussianBlur();

		Texture2D* blur(Texture2D* texture, RenderTarget2D* out, RenderTarget2D* cache);

		Texture2D* blurHalfResolution(Texture2D* texture, RenderTarget2D* out);
		Texture2D* blurQuarterResolution(Texture2D* texture, RenderTarget2D* out);
		Texture2D* blurEigthResolution(Texture2D* texture, RenderTarget2D* out);
		Texture2D* blurSixteenthResolution(Texture2D* texture, RenderTarget2D* out);

		void resize(unsigned width, unsigned height);

	protected:
		std::unique_ptr<GaussianBlurHorizontalShader> mHorizontalPass;
		std::unique_ptr<GaussianBlurVerticalShader> mVerticalPass;
		std::unique_ptr<RenderTarget2D>mHalfBlur;
		std::unique_ptr<RenderTarget2D>mQuarterBlur;
		std::unique_ptr<RenderTarget2D>mEigthBlur;
		std::unique_ptr<RenderTarget2D>mSixteenthBlur;
		std::unique_ptr<Sampler> mSampler;
	};
}
