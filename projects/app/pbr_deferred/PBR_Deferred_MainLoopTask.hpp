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

class PBR_Deferred_MainLoopTask
{
public:
	using RenderBackendPtr = RenderBackend*;
	typedef unsigned int uint;

	PBR_Deferred_MainLoopTask(RenderBackendPtr renderer);

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
	nex::LoggingClient logClient;
	float mixValue;
	Texture* panoramaSky;

	std::unique_ptr<PBR_Deferred> pbr_deferred;
	std::unique_ptr<PBR_GBuffer>  pbr_mrt;

	std::unique_ptr<SSAO_Deferred> ssao_deferred;
	std::unique_ptr<hbao::HBAO> hbao;

	RenderBackendPtr renderer;
	RenderTarget* renderTargetSingleSampled;
	Sprite screenSprite;
	DepthMap* shadowMap;
	bool showDepthMap;
};