#pragma once


namespace nex
{
	class RenderTarget2D;
	class Texture2D;
	class Shader;
	class GaussianBlur;
	class EquirectangularSkyBoxShader;
	class PanoramaSkyBoxShader;
	class SkyBoxShader;
	class DepthMapShader;
	class ShadowShader;
	class ScreenShader;
	class PostProcessor;
	class Sampler;

	class RenderTarget;

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


	class EffectLibrary {
	public:

		EffectLibrary(unsigned width, unsigned height);

		~EffectLibrary();

		// Inherited via EffectLibrary
		GaussianBlur* getGaussianBlur();

		EquirectangularSkyBoxShader* getEquirectangularSkyBoxShader();
		PanoramaSkyBoxShader* getPanoramaSkyBoxShader();
		SkyBoxShader* getSkyBoxShader();

		DepthMapShader* getDepthMapShader();

		ShadowShader* getShadowVisualizer();

		ScreenShader* getScreenShader();

		PostProcessor* getPostProcessor();

		DownSampler* getDownSampler();

		void resize(unsigned width, unsigned height);

	protected:
		std::unique_ptr<GaussianBlur> mGaussianBlur;
		std::unique_ptr<EquirectangularSkyBoxShader> mEquirectangualrSkyBox;
		std::unique_ptr<PanoramaSkyBoxShader> mPanoramaSkyBox;
		std::unique_ptr<SkyBoxShader> mSkyBox;
		std::unique_ptr<DepthMapShader> mDepthMap;
		std::unique_ptr<ShadowShader> mShadow;
		std::unique_ptr<ScreenShader> mScreen;
		std::unique_ptr<PostProcessor>mPostProcessor;
		std::unique_ptr<DownSampler>mDownSampler;
	};
}
