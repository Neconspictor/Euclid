#pragma once
#include <nex/logging/LoggingClient.hpp>
#include <nex/event/Task.hpp>
#include <nex/camera/Camera.hpp>
#include <nex/opengl/scene/SceneNode.hpp>
#include <nex/light/Light.hpp>
#include <nex/opengl/texture/Sprite.hpp>
#include <nex/opengl/shading_model/PBR_DeferredGL.hpp>
#include <nex/gui/ControllerStateMachine.hpp>
#include <nex/opengl/renderer/Renderer.hpp>
#include "nex/opengl/post_processing/AmbientOcclusion.hpp"

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
	hbao::HBAO_GL* getHBAO();
	AmbientOcclusionSelector* getAOSelector();

	PBR_DeferredGL* getPBR();


private:

	inline TextureGL * renderAO(Camera* camera, TextureGL* gPosition, TextureGL* gNormal);

	void drawSceneToCascade(SceneNode* scene);

	// Allow the UI mode classes accessing private members

	GaussianBlurGL* blurEffect;
	DirectionalLight globalLight;
	nex::LoggingClient logClient;
	float mixValue;
	TextureGL* panoramaSky;
	TextureGL* testTexture;

	std::unique_ptr<PBR_DeferredGL> m_pbr_deferred;
	std::unique_ptr<PBR_GBufferGL>  pbr_mrt;
	std::unique_ptr<CascadedShadowGL> m_cascadedShadow;

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