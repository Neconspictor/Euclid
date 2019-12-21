#pragma once

#include <nex/shader/Shader.hpp>


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
	class SpriteShader;
	class DepthSpriteShader;
	class PostProcessor;
	class DownSampler;
	class SimpleColorPass;
	class IrradianceSphereHullDrawPass;
	class Blit;
	class ViewSpaceZSpriteShader;

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

		SpriteShader* getSpritePass();
		DepthSpriteShader* getDepthSpritePass();
		ViewSpaceZSpriteShader* getViewSpaceZSpritePass();

		SimpleColorPass* getSimpleColorShader();
		IrradianceSphereHullDrawPass* getIrradianceSphereHullDrawShader();

		std::unique_ptr<SimpleColorMaterial> createSimpleColorMaterial();

		void resize(unsigned width, unsigned height);

	protected:
		std::unique_ptr<GaussianBlur> mGaussianBlur;
		std::unique_ptr<EquirectangularSkyBoxPass> mEquirectangualrSkyBox;
		std::unique_ptr<PanoramaSkyBoxPass> mPanoramaSkyBox;
		std::unique_ptr<SimpleColorPass> mSimpleColorShader;
		std::unique_ptr<IrradianceSphereHullDrawPass> mIrradianceSphereHullDrawShader;
		std::unique_ptr<SkyBoxPass> mSkyBox;
		std::unique_ptr<DepthMapPass> mDepthMap;
		std::unique_ptr<SpriteShader> mSprite;
		std::unique_ptr<DepthSpriteShader> mDepthSprite;
		std::unique_ptr<ViewSpaceZSpriteShader> mViewSpaceZSprite;
		std::unique_ptr<DownSampler>mDownSampler;
		std::unique_ptr<PostProcessor>mPostProcessor;
		std::unique_ptr<Blit> mBlit;
		
	};
}
