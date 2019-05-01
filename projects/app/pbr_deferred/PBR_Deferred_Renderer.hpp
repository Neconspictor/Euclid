#pragma once
#include <nex/Scene.hpp>
#include <nex/light/Light.hpp>
#include <nex/shadow/CascadedShadow.hpp>
#include <nex/texture/GBuffer.hpp>
#include <nex/post_processing/blur/GaussianBlur.hpp>
#include "nex/shader/Technique.hpp"
#include "nex/renderer/RenderCommandQueue.hpp"
#include "TesselationTest.hpp"

namespace nex
{

	class PbrDeferred;
	class AmbientOcclusionSelector;
	class RenderTarget2D;
	class AtmosphericScattering;
	class PbrProbe;
	class PbrForward;
	class Pbr;
	class Camera;

	class PBR_Deferred_Renderer
	{
	public:
		typedef unsigned int uint;

		PBR_Deferred_Renderer(RenderBackend* renderer,
			PbrDeferred* pbrDeferred,
			PbrForward* pbrForward,
			CascadedShadow* cascadedShadow,
			Input* input);

		bool getShowDepthMap() const;
		RenderCommandQueue* getCommandQueue();

		void init(int windowWidth, int windowHeight);
		void render(PerspectiveCamera* camera, DirectionalLight* sun, float frameTime, unsigned windowWidth, unsigned windowHeight);
		void setShowDepthMap(bool showDepthMap);
		void updateRenderTargets(unsigned width, unsigned height);

		AmbientOcclusionSelector* getAOSelector();
		TesselationTest* getTesselationTest();

	private:

		void renderShadows(PerspectiveCamera* camera, DirectionalLight* sun, Texture2D* depth);
		void renderDeferred(PerspectiveCamera* camera, DirectionalLight* sun, float frameTime, unsigned windowWidth, unsigned windowHeight);
		void renderForward(PerspectiveCamera* camera, DirectionalLight* sun, float frameTime, unsigned windowWidth, unsigned windowHeight);
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

		PbrDeferred* mPbrDeferred;
		PbrForward* mPbrForward;
		CascadedShadow* mCascadedShadow;
		RenderBackend* mRenderBackend;
		RenderCommandQueue mCommandQueue;

		TesselationTest mTesselationTest;
	};

	class PBR_Deferred_Renderer_ConfigurationView : public nex::gui::Drawable
	{
	public:
		PBR_Deferred_Renderer_ConfigurationView(PBR_Deferred_Renderer* renderer);

	protected:
		void drawSelf() override;

		PBR_Deferred_Renderer* mRenderer;
		gui::TesselationTest_Config mTesselationConfig;
	};
}
