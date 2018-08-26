#pragma once
#include <nex/logging/LoggingClient.hpp>
#include <nex/event/Task.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/scene/SceneNode.hpp>
#include <nex/light/Light.hpp>
#include <nex/sprite/Sprite.hpp>
#include <nex/post_processing/blur/GaussianBlur.hpp>
#include <nex/shading_model/PBR_Deferred.hpp>
#include <nex/post_processing/SSAO.hpp>
#include <nex/post_processing/HBAO.hpp>
#include <nex/gui/ControllerStateMachine.hpp>
#include <nex/renderer/Renderer.hpp>
#include "nex/post_processing/AmbientOcclusion.hpp"

class PBR_Deferred_Renderer : public Renderer
{
public:
	typedef unsigned int uint;

	PBR_Deferred_Renderer(Backend renderer);

	bool getShowDepthMap() const;
	void init(int windowWidth, int windowHeight);
	void render(SceneNode* scene, Camera* camera, float frameTime, int windowWidth, int windowHeight) override;
	void setShowDepthMap(bool showDepthMap);
	void updateRenderTargets(int width, int height);
	hbao::HBAO* getHBAO();
	AmbientOcclusionSelector* getAOSelector();

	PBR_Deferred* getPBR();


private:

	inline Texture * renderAO(Camera* camera, Texture* gPosition, Texture* gNormal);

	// Allow the UI mode classes accessing private members

	GaussianBlur* blurEffect;
	DirectionalLight globalLight;
	nex::LoggingClient logClient;
	float mixValue;
	Texture* panoramaSky;

	std::unique_ptr<PBR_Deferred> m_pbr_deferred;
	std::unique_ptr<PBR_GBuffer>  pbr_mrt;

	AmbientOcclusionSelector m_aoSelector;

	RenderTarget* renderTargetSingleSampled;
	Sprite screenSprite;
	DepthMap* shadowMap;
	bool showDepthMap;
};

class PBR_Deferred_Renderer_ConfigurationView : public nex::engine::gui::Drawable
{
public:
	PBR_Deferred_Renderer_ConfigurationView(PBR_Deferred_Renderer* renderer);

protected:
	void drawSelf() override;

	PBR_Deferred_Renderer* m_renderer;
};