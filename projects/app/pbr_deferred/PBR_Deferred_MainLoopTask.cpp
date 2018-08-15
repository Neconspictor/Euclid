#include <pbr_deferred/PBR_Deferred_MainLoopTask.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <glm/glm.hpp>
#include <mesh/SampleMeshes.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <camera/TrackballQuatCamera.hpp>
#include <camera/FPQuaternionCamera.hpp>
#include <shader/NormalsShader.hpp>
#include <camera/FPCamera.hpp>
#include <shader/PBRShader.hpp>
#include <shader/SkyBoxShader.hpp>
#include <scene/SceneNode.hpp>
#include <shader/DepthMapShader.hpp>
#include <shader/ScreenShader.hpp>
#include <shader/ShadowShader.hpp>
#include <util/Math.hpp>
#include <gui/Menu.hpp>
#include "gui/SceneGUI.hpp"
#include "gui/AppStyle.hpp"
#include "gui/ConfigurationWindow.hpp"

using namespace glm;
using namespace std;
using namespace platform;

int ssaaSamples = 1;

//misc/sphere.obj
//ModelManager::SKYBOX_MODEL_NAME
//misc/SkyBoxPlane.obj
PBR_Deferred_MainLoopTask::PBR_Deferred_MainLoopTask(EnginePtr engine,
	WindowPtr window,
	WindowSystemPtr windowSystem,
	RendererPtr renderer,
	GuiPtr gui,
	unsigned int flags) :
	Task(flags),
	blurEffect(nullptr),
	engine(engine),
	gui(gui),
	isRunning(true),
	logClient(getLogServer()),
	panoramaSky(nullptr),
	renderTargetSingleSampled(nullptr),
	runtime(0),
	renderer(renderer),
	scene(nullptr),
	shadowMap(nullptr),
	showDepthMap(false),
	ui(nullptr),
	uiModeStateMachine(nullptr),
	window(window),
	windowSystem(windowSystem)
{
	originalTitle = window->getTitle();
	logClient.setPrefix("[PBR_Deferred_MainLoopTask]");

	mixValue = 0.2f;

	camera = make_shared<FPCamera>(FPCamera());

	uiModeStateMachine.setUIMode(std::make_unique<GUI_Mode>(*this, *gui, std::unique_ptr<nex::engine::gui::View>()));

	style = make_unique<App::AppStyle>();
}

PBR_Deferred_MainLoopTask::~PBR_Deferred_MainLoopTask()
{
}

SceneNode* PBR_Deferred_MainLoopTask::createShadowScene()
{
	nodes.push_back(SceneNode());
	SceneNode* root = &nodes.back();

	nodes.push_back(SceneNode());
	SceneNode* ground = &nodes.back();
	root->addChild(ground);

	nodes.push_back(SceneNode());
	SceneNode* cube1 = &nodes.back();
	root->addChild(cube1);

	nodes.push_back(SceneNode());
	SceneNode* sphere = &nodes.back();
	root->addChild(sphere);

	vobs.push_back(Vob("misc/textured_plane.obj", Shaders::Pbr));
	ground->setVob(&vobs.back());
	//vobs.push_back(Vob("misc/textured_cube.obj"));
	vobs.push_back(Vob("normal_map_test/normal_map_test.obj", Shaders::Pbr));
	cube1->setVob(&vobs.back());

	vobs.push_back(Vob("normal_map_test/normal_map_sphere.obj", Shaders::Pbr));
	sphere->setVob(&vobs.back());

	ground->getVob()->setPosition({ 10, 0, 0 });
	cube1->getVob()->setPosition({ 0.0f, 1.3f, 0.0f });

	sphere->getVob()->setPosition({3.0f, 3.8f, -1.0f});

	return root;
}

SceneNode * PBR_Deferred_MainLoopTask::createCubeReflectionScene()
{
	nodes.push_back(SceneNode());
	SceneNode* root = &nodes.back();

	nodes.push_back(SceneNode());
	SceneNode* cube1 = &nodes.back();
	root->addChild(cube1);

	vobs.push_back(Vob("misc/untextured_cube.obj", Shaders::Pbr));
	cube1->setVob(&vobs.back());

	cube1->getVob()->setPosition({ 0.0f, 1.3f, 0.0f });
	return root;
}

Camera * PBR_Deferred_MainLoopTask::getCamera()
{
	return camera.get();
}

bool PBR_Deferred_MainLoopTask::getShowDepthMap() const
{
	return showDepthMap;
}

Timer * PBR_Deferred_MainLoopTask::getTimer()
{
	return &timer;
}

Window * PBR_Deferred_MainLoopTask::getWindow()
{
	return window;
}

void PBR_Deferred_MainLoopTask::init()
{
	using namespace placeholders;

	window->activate();

	int windowWidth = window->getWidth();
	int windowHeight = window->getHeight();

	ShaderManager* shaderManager = renderer->getShaderManager();
	ModelManager* modelManager = renderer->getModelManager();
	TextureManager* textureManager = renderer->getTextureManager();
	Input* input = window->getInputDevice();

	camera->setPosition(vec3(0.0f, 3.0f, 2.0f));
	camera->setLook(vec3(0.0f, 0.0f, -1.0f));
	camera->setUp(vec3(0.0f, 1.0f, 0.0f));
	camera->setAspectRatio((float)windowWidth / (float)windowHeight);

	Frustum frustum = camera->getFrustum(Perspective);
	frustum.left = -10.0f;
	frustum.right = 10.0f;
	frustum.bottom = -10.0f;
	frustum.top = 10.0f;
	frustum.nearPlane = 0.1f;
	frustum.farPlane = 10.0f;
	camera->setOrthoFrustum(frustum);


	if (TrackballQuatCamera* casted = dynamic_cast<TrackballQuatCamera*>(camera.get()))
	{
		auto cameraResizeCallback = bind(&TrackballQuatCamera::updateOnResize, casted, placeholders::_1, placeholders::_2);
		casted->updateOnResize(window->getWidth(), window->getHeight());
		input->addResizeCallback(cameraResizeCallback);
	}
	//auto rendererResizeCallback = bind(&Renderer::setViewPort, renderer, 0, 0, _1, _2);
	//window->addResizeCallback(rendererResizeCallback);


	shaderManager->loadShaders();

	modelManager->loadModels();

	window->activate();

	panoramaSky = textureManager->getHDRImage("skyboxes/panoramas/pisa.hdr", { false, true, Linear_Mipmap_Linear, Linear, ClampToEdge, RGB, true, BITS_32 });
	//panoramaSky = textureManager->getImage("skyboxes/panoramas/pisa.hdr", { true, true, Linear_Mipmap_Linear, Linear, ClampToEdge });
	//panoramaSky = textureManager->getHDRImage("hdr/newport_loft.hdr", { false, false, Linear, Linear, ClampToEdge, RGB, true, BITS_32 });

	//CubeMap* cubeMapSky = textureManager->createCubeMap("skyboxes/sky_right.jpg", "skyboxes/sky_left.jpg",
	//	"skyboxes/sky_top.jpg", "skyboxes/sky_bottom.jpg",
	//	"skyboxes/sky_back.jpg", "skyboxes/sky_front.jpg", true);
	
	SkyBoxShader* skyBoxShader = dynamic_cast<SkyBoxShader*>
		(shaderManager->getConfig(Shaders::SkyBox));

	PanoramaSkyBoxShader* panoramaSkyBoxShader = dynamic_cast<PanoramaSkyBoxShader*>
		(shaderManager->getConfig(Shaders::SkyBoxPanorama));

	EquirectangularSkyBoxShader* equirectangularSkyBoxShader = dynamic_cast<EquirectangularSkyBoxShader*>
		(shaderManager->getConfig(Shaders::SkyBoxEquirectangular));

	PBRShader* pbrShader = dynamic_cast<PBRShader*>
		(shaderManager->getConfig(Shaders::Pbr));

	shadowMap = renderer->createDepthMap(4096, 4096);
	renderTargetSingleSampled = renderer->createRenderTarget();

	panoramaSkyBoxShader->setSkyTexture(panoramaSky);
	equirectangularSkyBoxShader->setSkyTexture(panoramaSky);


	vec3 position = {1.0f, 1.0f, 1.0f };
	position = 5.0f * position;
	globalLight.setPosition(position);
	globalLight.lookAt({0,0,0});


	// init shaders
	pbrShader->setLightColor({ 1.0f, 1.0f, 1.0f });
	pbrShader->setLightDirection(globalLight.getLook());

	vec2 dim = {1.0, 1.0};
	vec2 pos = {0, 0};

	// center
	pos.x = 0.5f * (1.0f - dim.x);
	pos.y = 0.5f * (1.0f - dim.y);

	// align to bottom corner
	//pos.x = 1.0f - dim.x;
	//pos.y = 1.0f - dim.y;

	//align to top right corner
	//pos.x = 1.0f - dim.x;
	//pos.y = 0;

	screenSprite.setPosition(pos);
	screenSprite.setWidth(dim.x);
	screenSprite.setHeight(dim.y);
	//screenSprite.setZRotation(radians(45.0f));

	// init scene
	scene = createShadowScene();
	//scene = createCubeReflectionScene();
	//scene = createAsteriodField();

	blurEffect = renderer->getEffectLibrary()->getGaussianBlur();

	pbr_deferred = renderer->getShadingModelFactory().create_PBR_Deferred_Model(panoramaSky);
	pbr_mrt = pbr_deferred->createMultipleRenderTarget(windowWidth * ssaaSamples, windowHeight * ssaaSamples);

	ssao_deferred = renderer->createDeferredSSAO();
	hbao = renderer->createHBAO();


	using namespace nex::engine::gui;

	std::unique_ptr<SceneGUI> root = std::make_unique<SceneGUI>();
	std::unique_ptr<View> configurationWindow = std::make_unique<App::ConfigurationWindow>("Graphics Configuration Window", root->getMainMenuBar());
	std::unique_ptr<View> hbaoView = std::make_unique<hbao::HBAO_ConfigurationView>(hbao.get(), 
		root->getOptionMenu(), "HBAO##1");
	std::unique_ptr<View> hbaoView2 = std::make_unique<hbao::HBAO_ConfigurationView>(hbao.get(),
		root->getOptionMenu(), "HBAO##2");

	std::unique_ptr<View> hbaoView3 = std::make_unique<hbao::HBAO_ConfigurationView>(hbao.get(),
		root->getOptionMenu(), "HBAO##3");

	configurationWindow->useStyleClass(make_shared<App::ConfigurationStyle>());
	configurationWindow->addChild(move(hbaoView));
	configurationWindow->addChild(move(hbaoView2));
	configurationWindow->addChild(move(hbaoView3));
	root->addChild(move(configurationWindow));

	uiModeStateMachine.getUIMode()->setView(move(root));

	CubeMap* background = pbr_deferred->getEnvironmentMap();
	skyBoxShader->setSkyTexture(background);
	pbrShader->setSkyBox(background);

	uiModeStateMachine.init();

	style->apply();

	setupCallbacks();
}


void PBR_Deferred_MainLoopTask::onWindowsFocus(Window * window, bool receivedFocus)
{
	if (receivedFocus)
	{
		LOG(logClient, platform::Debug) << "received focus!";
		isRunning = true;
	}
	else
	{
		LOG(logClient, platform::Debug) << "lost focus!";
		isRunning = false;
		if (window->isInFullscreenMode())
		{
			window->minimize();
		}
	}
}



void PBR_Deferred_MainLoopTask::run()
{
	ModelDrawer* modelDrawer = renderer->getModelDrawer();
	ScreenShader* screenShader = dynamic_cast<ScreenShader*>(
		renderer->getShaderManager()->getConfig(Shaders::Screen));
	DepthMapShader* depthMapShader = dynamic_cast<DepthMapShader*>(
		renderer->getShaderManager()->getConfig(Shaders::DepthMap));
	using namespace chrono;
	
	float frameTime = timer.update();
	int millis = static_cast<int> (1.0f /(frameTime * 1000.0f));

	// manual 60 fps cap -> only temporary!
	int minimumMillis = 16;
	if (millis < minimumMillis)
	{
		//this_thread::sleep_for(milliseconds(minimumMillis - millis));
		//frameTime += timer.update();
	}

	float fps = counter.update(frameTime);
	updateWindowTitle(frameTime, fps);


	if (!window->isOpen())
	{
		engine->stop();
		return;
	}

	// Poll input events before checking if the app is running, otherwise 
	// the window is likely to hang or crash (at least on windows platform)
	windowSystem->pollEvents();
	
	// pause app if it is not active (e.g. window isn't focused)
	if (!isRunning)
	{
		this_thread::sleep_for(milliseconds(500));
		return;
	}


	window->activate();
	//handleInputEvents();
	uiModeStateMachine.frameUpdate();

	// update the scene
	scene->update(frameTime);



	//window->activate();
	//updateCamera(window->getInputDevice(), timer.getLastUpdateTimeDifference());

	//renderer->setViewPort(0, 0, window->getWidth(), window->getHeight());
	//renderer->useScreenTarget();
	//renderer->beginScene();
	//renderer->setBackgroundColor({0.5f, 0.5f, 0.5f});
	//renderer->endScene();
	
	FrustumCuboid cameraCuboid = camera->getFrustumCuboid(Perspective, 0.0f, 0.08f);
	const mat4& cameraView = camera->getView();
	mat4 inverseCameraView = inverse(cameraView);

	mat4 test = globalLight.getView();
	FrustumCuboid cameraCuboidWorld = test * inverseCameraView * cameraCuboid;
	AABB ccBB = fromCuboid(cameraCuboidWorld);

	Frustum shadowFrustum = { ccBB.min.x, ccBB.max.x, ccBB.min.y, ccBB.max.y, ccBB.min.z, ccBB.max.z };
	shadowFrustum = {-15.0f, 15.0f, -15.0f, 15.0f, -10.0f, 10.0f};
	globalLight.setOrthoFrustum(shadowFrustum);

	const mat4& lightProj = globalLight.getProjection(Orthographic);
	const mat4& lightView = globalLight.getView();


	// render scene to the shadow depth map
	renderer->useBaseRenderTarget(shadowMap);
	//renderer->beginScene();
	renderer->clearRenderTarget(shadowMap, RenderComponent::Depth);
	//renderer->enableAlphaBlending(false);
	//renderer->cullFaces(CullingMode::Back);

	pbr_deferred->drawSceneToShadowMap(scene,
		shadowMap,
		globalLight,
		lightView,
		lightProj);

	//renderer->cullFaces(CullingMode::Back);
	//renderer->endScene();

	renderer->useBaseRenderTarget(pbr_mrt.get());
	renderer->setViewPort(0, 0, window->getWidth() * ssaaSamples, window->getHeight() * ssaaSamples);
	//renderer->beginScene();
	renderer->clearRenderTarget(pbr_mrt.get(), RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);
		pbr_deferred->drawGeometryScene(scene,
			camera->getView(),
			camera->getPerspProjection());
	//renderer->endScene();

	//ssao_deferred->renderAO(pbr_mrt->getPosition(), pbr_mrt->getNormal(), camera->getPerspProjection());
	//ssao_deferred->blur();

	hbao::Projection projection;
	Frustum frustum = camera->getFrustum(Perspective);
	projection.fov = radians(camera->getFOV());
	projection.farplane = frustum.farPlane;
	projection.matrix = camera->getPerspProjection();
	projection.nearplane = frustum.nearPlane;
	projection.orthoheight = 0;
	projection.perspective = true;

	hbao->renderAO(pbr_mrt->getDepth(),  projection, true);

	Texture* aoTexture = hbao->getBlurredResult();

	// render scene to a offscreen buffer
	renderer->useBaseRenderTarget(renderTargetSingleSampled);
	renderer->setViewPort(0, 0, window->getWidth() * ssaaSamples, window->getHeight() * ssaaSamples);
	renderer->clearRenderTarget(renderTargetSingleSampled, RenderComponent::Color | RenderComponent::Depth | RenderComponent::Stencil);
	//renderer->clearRenderTarget(renderTargetSingleSampled, RenderComponent::Stencil);
	//renderer->beginScene();
	

	Dimension blitRegion = { 0,0, window->getWidth() * ssaaSamples, window->getHeight() * ssaaSamples };
	renderer->blitRenderTargets(pbr_mrt.get(),
		renderTargetSingleSampled,
		blitRegion,
		RenderComponent::Depth | RenderComponent::Stencil);

	//pbr_deferred->drawSky(camera->getPerspProjection(), camera->getView());

		//renderer->enableAlphaBlending(true);

		pbr_deferred->drawLighting(scene, 
			pbr_mrt.get(), 
			shadowMap->getTexture(), 
			aoTexture,
			globalLight, 
			camera->getView(), 
			lightProj * lightView);

		pbr_deferred->drawSky(camera->getPerspProjection(), camera->getView());
	


	//renderer->endScene();
	
	// finally render the offscreen buffer to a quad and do post processing stuff
	BaseRenderTarget* screenRenderTarget = renderer->getDefaultRenderTarget();

	renderer->useBaseRenderTarget(screenRenderTarget);
	renderer->setViewPort(0, 0, window->getWidth(), window->getHeight());
	//renderer->beginScene();
	renderer->clearRenderTarget(renderer->getDefaultRenderTarget(), RenderComponent::Depth | RenderComponent::Stencil);
	
	screenSprite.setTexture(renderTargetSingleSampled->getTexture());
	//screenSprite.setTexture(pbr_mrt->getAlbedo());
	//screenSprite.setTexture(ssao_deferred->getAO_Result());
	
	//screenSprite.setTexture(pbr_mrt->getAlbedo());

	depthMapShader->useDepthMapTexture(shadowMap->getTexture());

	screenShader->useTexture(screenSprite.getTexture());
	if (showDepthMap)
	{
		int width = window->getWidth();
		int height = window->getHeight();
		//renderer->setViewPort(width / 2 - 256, height / 2 - 256, 512, 512);
		//screenShader->useTexture(ssaoTexture);
		//modelDrawer->draw(&screenSprite, Shaders::Screen);
		//screenSprite.setTexture(shadowMap->getTexture());
		//depthMapShader->useDepthMapTexture(pbr_mrt->getDepth());
		//screenShader->useTexture(pbr_mrt->getDepth());
		//modelDrawer->draw(&screenSprite, Shaders::DepthMap);
		hbao->displayAOTexture();
		//hbao->displayTexture(pbr_mrt->getDepth());
	} else
	{
		modelDrawer->draw(&screenSprite, Shaders::Screen);
		//ssao_deferred->displayAOTexture();
	}
	//renderer->endScene();

	uiModeStateMachine.getUIMode()->render(*gui);

	// present rendered frame
	window->swapBuffers();

}

void PBR_Deferred_MainLoopTask::setShowDepthMap(bool showDepthMap)
{
	this->showDepthMap = showDepthMap;
}

void PBR_Deferred_MainLoopTask::setUI(SystemUI* ui)
{
	this->ui = ui;
}


void PBR_Deferred_MainLoopTask::setupCallbacks()
{
	using namespace placeholders;

	Input* input = window->getInputDevice();

	auto focusCallback = bind(&PBR_Deferred_MainLoopTask::onWindowsFocus, this, placeholders::_1, placeholders::_2);
	auto scrollCallback = bind(&Camera::onScroll, camera.get(), placeholders::_1, placeholders::_2);
	input->addWindowFocusCallback(focusCallback);
	input->addScrollCallback(scrollCallback);

	input->addResizeCallback([&](int width, int height)
	{
		LOG(logClient, Debug) << "addResizeCallback : width: " << width << ", height: " << height;

		if (!window->hasFocus()) {
			LOG(logClient, Debug) << "addResizeCallback : no focus!";
		}

		if (width == 0 || height == 0) {
			LOG(logClient, Warning) << "addResizeCallback : width or height is 0!";
			return;
		}

		camera->setAspectRatio((float)width / (float)height);

		//update render target dimension
		//the render target dimensions are dependent from the viewport size
		// so first update the viewport and than recreate the render targets
		// TODO, simplify this process -> render targets should be indepedent from viewport dimension?
		renderer->setViewPort(0, 0, width, height);
		renderer->destroyRenderTarget(renderTargetSingleSampled);
		renderTargetSingleSampled = renderer->createRenderTarget();
		pbr_mrt = pbr_deferred->createMultipleRenderTarget(width, height);
		ssao_deferred->onSizeChange(width, height);
		//hbao->onSizeChange(width, height);
	});

	input->addRefreshCallback([&]() {
		LOG(logClient, Warning) << "addRefreshCallback : called!";
		if (!window->hasFocus()) {
			LOG(logClient, Warning) << "addRefreshCallback : no focus!";
			return;
		}
	});
}

void PBR_Deferred_MainLoopTask::updateWindowTitle(float frameTime, float fps)
{
	runtime += frameTime;
	if (runtime > 1)
	{
		stringstream ss; ss << originalTitle << " : FPS= " << fps;
		window->setTitle(ss.str());
		runtime -= 1;
	}
}

BaseGUI_Mode::BaseGUI_Mode(PBR_Deferred_MainLoopTask & mainTask, ImGUI_Impl& guiRenderer, std::unique_ptr<nex::engine::gui::View> view) :
	UI_Mode(move(view)),
	mainTask(&mainTask),
	guiRenderer(&guiRenderer),
	logClient(getLogServer())
{
	logClient.setPrefix("[BaseGUI_Mode]");
}
/*
void BaseGUI_Mode::drawGUI()
{
	Window* window = mainTask->getWindow();

	// render GUI
	guiRenderer->newFrame();

	// 1. Show a simple window.
	// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug".
	{
		static float f = 0.0f;
		static int counter = 0;
		static bool show_app_simple_overlay = false;
		ImGui::Begin("", NULL, ImGuiWindowFlags_NoTitleBar);
		//ImGuiWindowFlags_NoTitleBar
		ImGui::Text("Hello, world!");                           // Display some text (you can use a format string too)
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f    
																//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

																//ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our windows open/close state
																//ImGui::Checkbox("Another Window", &show_another_window);

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (NB: most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::End();
	}

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit", "Esc"))
			{
				handleExitEvent();
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
			if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
			ImGui::Separator();
			if (ImGui::MenuItem("Cut", "CTRL+X")) {}
			if (ImGui::MenuItem("Copy", "CTRL+C")) {}
			if (ImGui::MenuItem("Paste", "CTRL+V")) {}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();

		if (ImGui::TreeNode("Tabbing"))
		{
			ImGui::Text("Use TAB/SHIFT+TAB to cycle through keyboard editable fields.");
			static char buf[32] = "dummy";
			ImGui::InputText("1", buf, IM_ARRAYSIZE(buf));
			ImGui::InputText("2", buf, IM_ARRAYSIZE(buf));
			ImGui::PushAllowKeyboardFocus(false);
			ImGui::InputText("4 (tab skip)", buf, IM_ARRAYSIZE(buf));
			//ImGui::SameLine(); ShowHelperMarker("Use ImGui::PushAllowKeyboardFocus(bool)\nto disable tabbing through certain widgets.");
			ImGui::PopAllowKeyboardFocus();
			ImGui::InputText("5", buf, IM_ARRAYSIZE(buf));
			ImGui::TreePop();
		}
	}

	//UI_Mode::drawGUI();

	ImGui::Render();
	guiRenderer->renderDrawData(ImGui::GetDrawData());
}
*/

void BaseGUI_Mode::frameUpdate(UI_ModeStateMachine & stateMachine)
{
	using namespace platform;

	Input* input = mainTask->getWindow()->getInputDevice();
	Window* window = mainTask->getWindow();

	if (input->isPressed(Input::KEY_ESCAPE))
	{
		handleExitEvent();
	}

	if (input->isPressed(Input::KEY_Y))
	{
		mainTask->setShowDepthMap(!mainTask->getShowDepthMap());
	}


	// Context refresh Does not work right now!
	if (input->isPressed(Input::KEY_B)) {
		if (window->isInFullscreenMode()) {
			window->setWindowed();
		}
		else {
			window->setFullscreen();
		}

		LOG(logClient, Debug) << "toggle";
	}
}

void BaseGUI_Mode::init()
{
}

void BaseGUI_Mode::handleExitEvent()
{
	mainTask->getWindow()->close();
}


GUI_Mode::GUI_Mode(PBR_Deferred_MainLoopTask & mainTask, ImGUI_Impl& guiRenderer, std::unique_ptr<nex::engine::gui::View> view) : 
	BaseGUI_Mode(mainTask, guiRenderer, move(view))
{
	logClient.setPrefix("[GUI_Mode]");
	mainTask.getWindow()->showCursor(false);
}

void GUI_Mode::frameUpdate(UI_ModeStateMachine & stateMachine)
{
	BaseGUI_Mode::frameUpdate(stateMachine);
	//std::cout << "GUI_Mode::frameUpdate(UI_ModeStateMachine &) called!" << std::endl;
	Input* input = mainTask->getWindow()->getInputDevice();

	// Switch to camera mode?
	if (input->isPressed(Input::KEY_C)) {
		stateMachine.setUIMode(std::make_unique<CameraMode>(*mainTask, *guiRenderer, move(m_view)));
	}
}

CameraMode::CameraMode(PBR_Deferred_MainLoopTask & mainTask, ImGUI_Impl& guiRenderer, std::unique_ptr<nex::engine::gui::View> view) :
	BaseGUI_Mode(mainTask, guiRenderer, move(view))
{
	logClient.setPrefix("[CameraMode]");
	mainTask.getWindow()->showCursor(true);
}

void CameraMode::frameUpdate(UI_ModeStateMachine & stateMachine)
{
	BaseGUI_Mode::frameUpdate(stateMachine);

	//std::cout << "CameraMode::frameUpdate(UI_ModeStateMachine &) called!" << std::endl;
	Input* input = mainTask->getWindow()->getInputDevice();
	float frameTime = mainTask->getTimer()->getLastUpdateTimeDifference();

	// update camera
	updateCamera(input, frameTime);

	// Switch to gui mode?
	if (input->isPressed(Input::KEY_C)) {
		stateMachine.setUIMode(std::make_unique<GUI_Mode>(*mainTask, *guiRenderer, move(m_view)));
	}
}

void CameraMode::updateCamera(Input * input, float deltaTime)
{
	Camera* camera = mainTask->getCamera();
	Window* window = mainTask->getWindow();

	camera->update(input, deltaTime);
	window->setCursorPosition(window->getWidth() / 2, window->getHeight() / 2);
}