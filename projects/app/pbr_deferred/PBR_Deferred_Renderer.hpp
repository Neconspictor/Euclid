#pragma once
#include <nex/event/Task.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/SceneNode.hpp>
#include <nex/light/Light.hpp>
#include <nex/shadow/CascadedShadow.hpp>
#include <nex/texture/GBuffer.hpp>
#include <nex/post_processing/blur/GaussianBlur.hpp>
#include <nex/Renderer.hpp>

namespace nex
{

	class PbrDeferred;
	class AmbientOcclusionSelector;
	class RenderTarget2D;
	class AtmosphericScattering;
	class PbrProbe;
	class PbrForward;

	class PBR_Deferred_Renderer : public Renderer
	{
	public:
		typedef unsigned int uint;

		PBR_Deferred_Renderer(RenderBackend* renderer, Input* input);

		bool getShowDepthMap() const;
		void init(int windowWidth, int windowHeight);
		void render(SceneNode* scene, Camera* camera, float frameTime, int windowWidth, int windowHeight) override;
		void setShowDepthMap(bool showDepthMap);
		void updateRenderTargets(int width, int height);

		AmbientOcclusionSelector* getAOSelector();

		CascadedShadow* getCSM();

		PbrDeferred* getPBR();


	private:

		void renderDeferred(SceneNode* scene, Camera* camera, float frameTime, int windowWidth, int windowHeight);
		void renderForward(SceneNode* scene, Camera* camera, float frameTime, int windowWidth, int windowHeight);

		std::unique_ptr<RenderTarget2D> createLightingTarget(unsigned width, unsigned height);

		// Allow the UI mode classes accessing private members

		GaussianBlur* blurEffect;
		DirectionalLight globalLight;
		nex::Logger m_logger;
		Texture* panoramaSky;
		Texture* testTexture;
		Texture2D* smaaTestTex;
		Texture2D* smaaTestSRGBTex;

		std::unique_ptr<PbrProbe> mPbrProbe;
		std::unique_ptr<PbrDeferred> mPbrDeferred;
		std::unique_ptr<PBR_GBuffer>  mPbrMrt;
		std::unique_ptr<CascadedShadow> mCascadedShadow;

		// forward
		std::unique_ptr<PbrForward> mPbrForward;

		std::unique_ptr<RenderTarget2D> mRenderTargetSingleSampled;
		std::unique_ptr<RenderTarget2D> mPingPong;

		std::unique_ptr<AtmosphericScattering> mAtmosphericScattering;
		//DepthMap* shadowMap;
		bool mShowDepthMap;
		Input* mInput;
	};

	class PBR_Deferred_Renderer_ConfigurationView : public nex::gui::Drawable
	{
	public:
		PBR_Deferred_Renderer_ConfigurationView(PBR_Deferred_Renderer* renderer);

	protected:
		void drawSelf() override;

		PBR_Deferred_Renderer* mRenderer;
	};
}