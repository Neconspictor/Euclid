#pragma once
#include <nex/event/Task.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/SceneNode.hpp>
#include <nex/light/Light.hpp>
#include <nex/texture/Sprite.hpp>
#include <nex/pbr/PBR_Deferred.hpp>
#include <nex/gui/ControllerStateMachine.hpp>
#include <nex/post_processing/AmbientOcclusion.hpp>
#include <nex/shadow/CascadedShadow.hpp>
#include <nex/texture/GBuffer.hpp>
#include <nex/pbr/PBR_Deferred.hpp>
#include <nex/post_processing/blur/GaussianBlur.hpp>
#include <nex/Renderer.hpp>
#include <nex/RenderBackend.hpp>
#include <nex/sky/AtmosphericScattering.hpp>
#include "SceneNearFarComputeShader.hpp"

namespace nex
{
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
		nex::HBAO* getHBAO();
		AmbientOcclusionSelector* getAOSelector();

		CascadedShadow* getCSM();

		PBR_Deferred* getPBR();


	private:

		Texture * renderAO(Camera* camera, Texture* gDepth, Texture* gNormal);

		std::unique_ptr<RenderTarget2D> createLightingTarget(unsigned width, unsigned height);

		glm::vec2 computeNearFarTest(Camera* camera, int windowWidth, int windowHeight, Texture* depth);

		// Allow the UI mode classes accessing private members

		GaussianBlur* blurEffect;
		DirectionalLight globalLight;
		nex::Logger m_logger;
		Texture* panoramaSky;
		Texture* testTexture;

		std::unique_ptr<PBR_Deferred> m_pbr_deferred;
		std::unique_ptr<PBR_GBuffer>  pbr_mrt;
		std::unique_ptr<CascadedShadow> m_cascadedShadow;

		AmbientOcclusionSelector m_aoSelector;

		std::unique_ptr<RenderTarget2D> renderTargetSingleSampled;

		AtmosphericScattering mAtmosphericScattering;

		Sprite screenSprite;
		//DepthMap* shadowMap;
		bool showDepthMap;

		std::unique_ptr<SceneNearFarComputeShader> mSceneNearFarComputeShader;
		Input* mInput;
	};

	class PBR_Deferred_Renderer_ConfigurationView : public nex::gui::Drawable
	{
	public:
		PBR_Deferred_Renderer_ConfigurationView(PBR_Deferred_Renderer* renderer);

	protected:
		void drawSelf() override;

		PBR_Deferred_Renderer* m_renderer;
	};
}