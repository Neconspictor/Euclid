#pragma once
#include <platform/logging/LoggingClient.hpp>
#include <platform/event/Task.hpp>
#include <renderer/Renderer3D.hpp>
#include <camera/Camera.hpp>
#include <scene/SceneNode.hpp>
#include <light/Light.hpp>
#include <sprite/Sprite.hpp>
#include <post_processing/blur/GaussianBlur.hpp>
#include <shading_model/PBR_Deferred.hpp>
#include <post_processing/SSAO.hpp>
#include <post_processing/HBAO.hpp>
#include <gui/ControllerStateMachine.hpp>

class PBR_Deferred_MainLoopTask
{
public:
	using RendererPtr = Renderer3D*;
	typedef unsigned int uint;

	PBR_Deferred_MainLoopTask(RendererPtr renderer);

	bool getShowDepthMap() const;
	void init(int windowWidth, int windowHeight);
	void run(SceneNode* scene, Camera* camera, float frameTime, int windowWidth, int windowHeight);
	void setShowDepthMap(bool showDepthMap);
	void setRunning(bool isRunning);
	bool isRunning() const;
	void updateRenderTargets(int width, int height);
	hbao::HBAO* getHBAO();

private:

	// Allow the UI mode classes accessing private members

	GaussianBlur* blurEffect;
	DirectionalLight globalLight;
	bool m_isRunning;
	platform::LoggingClient logClient;
	float mixValue;
	Texture* panoramaSky;

	std::unique_ptr<PBR_Deferred> pbr_deferred;
	std::unique_ptr<PBR_GBuffer>  pbr_mrt;

	std::unique_ptr<SSAO_Deferred> ssao_deferred;
	std::unique_ptr<hbao::HBAO> hbao;

	RendererPtr renderer;
	RenderTarget* renderTargetSingleSampled;
	Sprite screenSprite;
	DepthMap* shadowMap;
	bool showDepthMap;
};