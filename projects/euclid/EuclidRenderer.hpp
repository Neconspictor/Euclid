#pragma once
#include <nex/scene/Scene.hpp>
#include <nex/light/Light.hpp>
#include <nex/shadow/CascadedShadow.hpp>
#include <nex/texture/GBuffer.hpp>
#include <nex/post_processing/blur/GaussianBlur.hpp>
#include "nex/renderer/RenderCommandQueue.hpp"
#include <nex/terrain/TesselationTest.hpp>
#include <nex/renderer/Renderer.hpp>
#include <nex/texture/RenderTarget.hpp>
#include <nex/water/Ocean.hpp>
#include <nex/GI/ProbeSelector.hpp>

namespace nex
{
	class PbrTechnique;
	class PbrDeferred;
	class AmbientOcclusionSelector;
	class RenderTarget2D;
	class AtmosphericScattering;
	class Probe;
	class PbrForward;
	class Pbr;
	class Camera;

	class EuclidRenderer : public Renderer
	{
	public:
		typedef unsigned int uint;

		EuclidRenderer(RenderBackend* renderer,
			PbrTechnique* pbrTechnique,
			CascadedShadow* cascadedShadow,
			AtmosphericScattering* atmosphericScattering,
			Input* input);

		virtual ~EuclidRenderer();

		bool getShowDepthMap() const;
		bool getIrradianceAA() const;
		bool getBlurIrradiance() const;
		ProbeSelectionAlgorithm getProbeSelectionAlg() const;
		bool getRenderGIinHalfRes() const;
		bool getDownSampledDepth() const;

		void init(int windowWidth, int windowHeight);

		virtual void render(const RenderCommandQueue& queue, 
			const RenderContext& constants,
			bool postProcess,
			RenderTarget* out = nullptr) override;

		void setShowDepthMap(bool showDepthMap);
		void setIrradianceAA(bool antialias);
		void setBlurIrradiance(bool value);
		void setProbeSelectionAlg(ProbeSelectionAlgorithm alg);
		void setRenderGIinHalfRes(bool value);
		void setDownsampledDepth(bool value);

		virtual void updateRenderTargets(unsigned width, unsigned height) override;

		AmbientOcclusionSelector* getAOSelector();
		PBR_GBuffer* getGbuffer();
		TesselationTest* getTesselationTest();
		CascadedShadow* getCascadedShadow(); 

		void pushDepthFunc(std::function<void()> func) override;


		
		RenderTarget* getActiveIrradianceAmbientReflectionRT() override;
		RenderTarget* getOutRT() override;
		const Texture* getOutStencilView() override;
		RenderTarget* getPingPongRT() override;
		const Texture* getPingPongStencilView() override;


		void renderShadows(const nex::RenderCommandQueue::Buffer& shadowCommands, const RenderContext& constants, const DirLight& sun, Texture2D* depth);

	private:
		
		void renderDeferred(const RenderCommandQueue& queue, const RenderContext& constants, const DirLight& sun);
		void renderForward(const RenderCommandQueue& queue, const RenderContext& constants, const DirLight& sun);
		void renderSky(const RenderContext& constants, const DirLight& sun);

		std::unique_ptr<RenderTarget> createLightingTarget(unsigned width, unsigned height, const PBR_GBuffer* gBuffer);

		// Allow the UI mode classes accessing private members

		GaussianBlur* blurEffect;
		nex::Logger m_logger;

		std::unique_ptr<PBR_GBuffer>  mPbrMrt;

		
		std::unique_ptr<RenderTarget> mIrradianceAmbientReflectionRT[2];
		std::unique_ptr<RenderTarget> mDepthHalf;
		std::unique_ptr<RenderTarget2D> mPingPong;
		std::unique_ptr<Texture> mPingPongStencilView;
		std::unique_ptr<Texture> mOutStencilView;
		std::unique_ptr<Texture> mWaterMinDepth;
		std::unique_ptr<Texture> mWaterMaxDepth;
		std::unique_ptr<RenderTarget2D> mPingPongHalf;
		std::unique_ptr<RenderTarget> mOutRT;
		RenderTargetSwitcher mOutSwitcherTAA;

		//DepthMap* shadowMap;
		bool mShowDepthMap;
		Input* mInput;

		AtmosphericScattering* mAtmosphericScattering;
		CascadedShadow* mCascadedShadow;
		RenderBackend* mRenderBackend;
		
		TesselationTest mTesselationTest;

		std::vector<std::function<void()>> mDepthFuncs;

		bool mAntialiasIrradiance;
		bool mBlurIrradiance;
		bool mRenderGIinHalfRes;
		bool mUseDownSampledDepth;
		bool mActiveIrradianceRT;
		ProbeSelectionAlgorithm mProbeSelectionAlg = ProbeSelectionAlgorithm::FOUR_NEAREST_INTERPOLATION;
	};
}
