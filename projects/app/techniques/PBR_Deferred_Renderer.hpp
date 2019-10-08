#pragma once
#include <nex/Scene.hpp>
#include <nex/light/Light.hpp>
#include <nex/shadow/CascadedShadow.hpp>
#include <nex/texture/GBuffer.hpp>
#include <nex/post_processing/blur/GaussianBlur.hpp>
#include "nex/shader/Technique.hpp"
#include "nex/renderer/RenderCommandQueue.hpp"
#include "TesselationTest.hpp"
#include "Ocean.hpp"
#include <nex/renderer/Renderer.hpp>

namespace nex
{
	class PbrTechnique;
	class PbrDeferred;
	class AmbientOcclusionSelector;
	class RenderTarget2D;
	class AtmosphericScattering;
	class PbrProbe;
	class PbrForward;
	class Pbr;
	class Camera;

	class PBR_Deferred_Renderer : public Renderer
	{
	public:
		typedef unsigned int uint;

		PBR_Deferred_Renderer(RenderBackend* renderer,
			PbrTechnique* pbrTechnique,
			CascadedShadow* cascadedShadow,
			Input* input);

		virtual ~PBR_Deferred_Renderer();

		bool getShowDepthMap() const;
		bool getIrradianceAA() const;
		bool getBlurIrradiance() const;
		bool getRenderGIinHalfRes() const;
		bool getDownSampledDepth() const;

		void init(int windowWidth, int windowHeight);

		virtual void render(const RenderCommandQueue& queue, 
			const Camera&  camera, const DirLight& sun,
			unsigned viewportWidth, 
			unsigned viewportHeight, 
			bool postProcess,
			RenderTarget* out) override;

		void setShowDepthMap(bool showDepthMap);
		void setIrradianceAA(bool antialias);
		void setBlurIrradiance(bool value);
		void setRenderGIinHalfRes(bool value);
		void setDownsampledDepth(bool value);

		virtual void updateRenderTargets(unsigned width, unsigned height) override;

		AmbientOcclusionSelector* getAOSelector();
		PBR_GBuffer* getGbuffer();
		TesselationTest* getTesselationTest();
		Ocean* getOcean();
		CascadedShadow* getCascadedShadow(); 

		void pushDepthFunc(std::function<void()> func) override;

		RenderTarget* getOutRendertTarget() override;


		void renderShadows(const nex::RenderCommandQueue::Buffer& shadowCommands, const Pass::Constants& constants, const DirLight& sun, Texture2D* depth);
		
	private:
		
		void renderDeferred(const RenderCommandQueue& queue, const Pass::Constants& constants, const DirLight& sun);
		void renderForward(const RenderCommandQueue& queue, const Pass::Constants& constants, const DirLight& sun);
		void renderSky(const Pass::Constants& constants, const DirLight& sun);

		std::unique_ptr<RenderTarget2D> createLightingTarget(unsigned width, unsigned height, const PBR_GBuffer* gBuffer);

		// Allow the UI mode classes accessing private members

		GaussianBlur* blurEffect;
		nex::Logger m_logger;

		std::unique_ptr<PBR_GBuffer>  mPbrMrt;

		std::unique_ptr<RenderTarget2D> mRenderTargetSingleSampled;
		std::unique_ptr<RenderTarget> mIrradianceAmbientReflectionRT[2];
		std::unique_ptr<RenderTarget> mDepthHalf;
		std::unique_ptr<RenderTarget2D> mPingPong;
		std::unique_ptr<RenderTarget2D> mPingPongHalf;
		std::unique_ptr<RenderTarget2D> mOutRT;

		std::unique_ptr<AtmosphericScattering> mAtmosphericScattering;
		//DepthMap* shadowMap;
		bool mShowDepthMap;
		Input* mInput;

		CascadedShadow* mCascadedShadow;
		RenderBackend* mRenderBackend;
		
		TesselationTest mTesselationTest;

		OceanGPU mOcean;

		std::vector<std::function<void()>> mDepthFuncs;

		bool mAntialiasIrradiance;
		bool mBlurIrradiance;
		bool mRenderGIinHalfRes;
		bool mUseDownSampledDepth;
		bool mActiveIrradianceRT;
	};

	class PBR_Deferred_Renderer_ConfigurationView : public nex::gui::Drawable
	{
	public:
		PBR_Deferred_Renderer_ConfigurationView(PBR_Deferred_Renderer* renderer);

	protected:
		void drawSelf() override;

		PBR_Deferred_Renderer* mRenderer;
		gui::TesselationTest_Config mTesselationConfig;
		gui::OceanConfig mOceanConfig;
	};
}
