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

		void init(int windowWidth, int windowHeight);

		virtual void render(const RenderCommandQueue& queue, 
			PerspectiveCamera* camera, 
			DirectionalLight* sun, 
			unsigned viewportWidth, 
			unsigned viewportHeight, 
			RenderTarget* out) override;

		void setShowDepthMap(bool showDepthMap);
		void updateRenderTargets(unsigned width, unsigned height);

		AmbientOcclusionSelector* getAOSelector();
		TesselationTest* getTesselationTest();
		Ocean* getOcean();

	private:

		void renderShadows(const nex::RenderCommandQueue::Buffer& shadowCommands, PerspectiveCamera* camera, DirectionalLight* sun, Texture2D* depth);
		void renderDeferred(const RenderCommandQueue& queue, PerspectiveCamera* camera, DirectionalLight* sun, unsigned windowWidth, unsigned windowHeight);
		void renderForward(const RenderCommandQueue& queue, PerspectiveCamera* camera, DirectionalLight* sun, unsigned windowWidth, unsigned windowHeight);
		void renderSky(PerspectiveCamera* camera, DirectionalLight* sun, unsigned width, unsigned height);

		std::unique_ptr<RenderTarget2D> createLightingTarget(unsigned width, unsigned height, const PBR_GBuffer* gBuffer);

		// Allow the UI mode classes accessing private members

		GaussianBlur* blurEffect;
		nex::Logger m_logger;

		std::unique_ptr<PBR_GBuffer>  mPbrMrt;

		std::unique_ptr<RenderTarget2D> mRenderTargetSingleSampled;
		std::unique_ptr<RenderTarget2D> mPingPong;

		std::unique_ptr<AtmosphericScattering> mAtmosphericScattering;
		//DepthMap* shadowMap;
		bool mShowDepthMap;
		Input* mInput;

		PbrTechnique* mPbrTechnique;
		CascadedShadow* mCascadedShadow;
		RenderBackend* mRenderBackend;
		
		TesselationTest mTesselationTest;

		OceanGPU mOcean;
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
