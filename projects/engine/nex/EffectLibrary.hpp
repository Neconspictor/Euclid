#pragma once


namespace nex
{
	class RenderTarget2D;
	class Texture2D;
	class Shader;
	class GaussianBlur;
	class EquirectangularSkyBoxPass;
	class PanoramaSkyBoxPass;
	class SkyBoxPass;
	class DepthMapPass;
	class ShadowPass;
	class SpritePass;
	class DepthSpritePass;
	class PostProcessor;
	class DownSampler;
	class SimpleColorTechnique;
	class IrradianceSphereHullDrawTechnique;
	class Blit;

	class RenderTarget;
	class Technique;
	class SimpleColorMaterial;
	


	class EffectLibrary {
	public:

		EffectLibrary(unsigned width, unsigned height);

		virtual ~EffectLibrary();

		Blit* getBlit();

		// Inherited via EffectLibrary
		GaussianBlur* getGaussianBlur();

		EquirectangularSkyBoxPass* getEquirectangularSkyBoxShader();
		PanoramaSkyBoxPass* getPanoramaSkyBoxShader();
		SkyBoxPass* getSkyBoxShader();

		DepthMapPass* getDepthMapShader();

		DownSampler* getDownSampler();

		PostProcessor* getPostProcessor();

		SpritePass* getSpritePass();
		DepthSpritePass* getDepthSpritePass();

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
		std::unique_ptr<SpritePass> mSprite;
		std::unique_ptr<DepthSpritePass> mDepthSprite;
		std::unique_ptr<DownSampler>mDownSampler;
		std::unique_ptr<PostProcessor>mPostProcessor;
		std::unique_ptr<Blit> mBlit;
		
	};
}
