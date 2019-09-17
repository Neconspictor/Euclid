#pragma once


namespace nex
{
	class RenderTarget2D;
	class Texture2D;
	class Pass;
	class GaussianBlur;
	class EquirectangularSkyBoxPass;
	class PanoramaSkyBoxPass;
	class SkyBoxPass;
	class DepthMapPass;
	class ShadowPass;
	class ScreenPass;
	class PostProcessor;
	class DownSampler;
	class SimpleColorTechnique;
	class IrradianceSphereHullDrawTechnique;

	class RenderTarget;
	class Technique;
	class SimpleColorMaterial;


	class EffectLibrary {
	public:

		EffectLibrary(unsigned width, unsigned height);

		virtual ~EffectLibrary();

		// Inherited via EffectLibrary
		GaussianBlur* getGaussianBlur();

		EquirectangularSkyBoxPass* getEquirectangularSkyBoxShader();
		PanoramaSkyBoxPass* getPanoramaSkyBoxShader();
		SkyBoxPass* getSkyBoxShader();

		DepthMapPass* getDepthMapShader();

		DownSampler* getDownSampler();

		PostProcessor* getPostProcessor();

		ScreenPass* getScreenShader();

		SimpleColorTechnique* getSimpleColorTechnique();
		IrradianceSphereHullDrawTechnique* getIrradianceSphereHullDrawTechnique();

		std::unique_ptr<SimpleColorMaterial> createSimpleColorMaterial();

		void resize(unsigned width, unsigned height);

	protected:
		std::unique_ptr<GaussianBlur> mGaussianBlur;
		std::unique_ptr<EquirectangularSkyBoxPass> mEquirectangualrSkyBox;
		std::unique_ptr<PanoramaSkyBoxPass> mPanoramaSkyBox;
		std::unique_ptr<SimpleColorTechnique> mSimpleColorTechnique;
		std::unique_ptr<IrradianceSphereHullDrawTechnique> mIrradianceSphereHullDrawTechnique;
		std::unique_ptr<SkyBoxPass> mSkyBox;
		std::unique_ptr<DepthMapPass> mDepthMap;
		std::unique_ptr<ScreenPass> mScreen;
		std::unique_ptr<DownSampler>mDownSampler;
		std::unique_ptr<PostProcessor>mPostProcessor;
		
	};
}
