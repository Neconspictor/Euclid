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

	class RenderTarget;

	class DownSampler
	{
	public:

		DownSampler();

		~DownSampler();

		/**
		 * Renders a texture to the 
		 */
		void downsample(Texture2D* src, RenderTarget2D* dest);

	private:

		class DownSampleShader;
		std::unique_ptr<DownSampleShader> mDownSampleShader;
		std::unique_ptr<RenderTarget> mHalfResolution;
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