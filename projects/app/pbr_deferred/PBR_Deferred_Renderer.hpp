#pragma once
#include <nex/logging/LoggingClient.hpp>
#include <nex/event/Task.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/opengl/scene/SceneNode.hpp>
#include <nex/light/Light.hpp>
#include <nex/sprite/Sprite.hpp>
#include <nex/opengl/shading_model/PBR_DeferredGL.hpp>
#include <nex/post_processing/SSAO.hpp>
#include <nex/post_processing/HBAO.hpp>
#include <nex/gui/ControllerStateMachine.hpp>
#include <nex/post_processing/AmbientOcclusion.hpp>
#include <nex/shadowing/CascadedShadow.hpp>
#include "nex/opengl/renderer/Renderer.hpp"

class PBR_Deferred_Renderer : public Renderer
{
public:
	typedef unsigned int uint;

	PBR_Deferred_Renderer(RendererOpenGL* renderer);

	bool getShowDepthMap() const;
	void init(int windowWidth, int windowHeight);
	void render(SceneNode* scene, Camera* camera, float frameTime, int windowWidth, int windowHeight) override;
	void setShowDepthMap(bool showDepthMap);
	void updateRenderTargets(int width, int height);
	hbao::HBAO* getHBAO();
	AmbientOcclusionSelector* getAOSelector();

	PBR_DeferredGL* getPBR();


private:

	inline Texture * renderAO(Camera* camera, Texture* gPosition, Texture* gNormal);

	void drawSceneToCascade(SceneNode* scene);

	// Allow the UI mode classes accessing private members

	GaussianBlurGL* blurEffect;
	DirectionalLight globalLight;
	nex::LoggingClient logClient;
	float mixValue;
	TextureGL* panoramaSky;

	std::unique_ptr<PBR_DeferredGL> m_pbr_deferred;
	std::unique_ptr<PBR_GBufferGL>  pbr_mrt;
	std::unique_ptr<CascadedShadow> m_cascadedShadow;

	AmbientOcclusionSelector m_aoSelector;

	RenderTargetGL* renderTargetSingleSampled;
	Sprite screenSprite;
	DepthMapGL* shadowMap;
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