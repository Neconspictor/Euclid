#pragma once
#include <nex/event/Task.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/SceneNode.hpp>
#include <nex/light/Light.hpp>
#include <nex/shadow/CascadedShadow.hpp>
#include <nex/texture/GBuffer.hpp>
#include <nex/post_processing/blur/GaussianBlur.hpp>
#include <nex/Renderer.hpp>
#include "nex/shader/Technique.hpp"

namespace nex
{

	class PbrDeferred;
	class AmbientOcclusionSelector;
	class RenderTarget2D;
	class AtmosphericScattering;
	class PbrProbe;
	class PbrForward;
	class Pbr;

	class PBR_Deferred_Renderer : public Renderer
	{
	public:
		typedef unsigned int uint;

		PBR_Deferred_Renderer(RenderBackend* renderer,
			PbrDeferred* pbrDeferred,
			PbrForward* pbrForward,
			CascadedShadow* cascadedShadow,
			Input* input);

		bool getShowDepthMap() const;
		void init(int windowWidth, int windowHeight);
		void render(SceneNode* scene, Camera* camera, DirectionalLight* sun, float frameTime, unsigned windowWidth, unsigned windowHeight) override;
		void setShowDepthMap(bool showDepthMap);
		void updateRenderTargets(unsigned width, unsigned height);

		AmbientOcclusionSelector* getAOSelector();

	private:

		void renderShadows(SceneNode* scene, Camera* camera, DirectionalLight* sun, Texture2D* depth);
		void renderDeferred(SceneNode* scene, Camera* camera, DirectionalLight* sun, float frameTime, unsigned windowWidth, unsigned windowHeight);
		void renderForward(SceneNode* scene, Camera* camera, DirectionalLight* sun, float frameTime, unsigned windowWidth, unsigned windowHeight);
		void renderSky(Camera* camera, DirectionalLight* sun, unsigned width, unsigned height);

		std::unique_ptr<RenderTarget2D> createLightingTarget(unsigned width, unsigned height);

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
