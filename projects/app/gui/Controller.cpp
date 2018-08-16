#include <platform/Window.hpp>
#include <gui/Controller.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <pbr_deferred/PBR_Deferred_MainLoopTask.hpp>


App::BaseController::BaseController(PBR_Deferred_MainLoopTask & mainTask, ImGUI_Impl& guiRenderer, std::unique_ptr<nex::engine::gui::Drawable> drawable) :
	Controller(move(drawable)),
	mainTask(&mainTask),
	guiRenderer(&guiRenderer),
	logClient(platform::getLogServer())
{
	logClient.setPrefix("[BaseController]");
}

/*
void BaseController::drawGUI()
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

//Controller::drawGUI();

ImGui::Render();
guiRenderer->renderDrawData(ImGui::GetDrawData());
}
*/

void App::BaseController::frameUpdate(ControllerStateMachine & stateMachine)
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

void App::BaseController::init()
{
}

void App::BaseController::handleExitEvent()
{
	mainTask->getWindow()->close();
}


App::EditMode::EditMode(PBR_Deferred_MainLoopTask & mainTask, ImGUI_Impl& guiRenderer, std::unique_ptr<nex::engine::gui::Drawable> drawable) :
	BaseController(mainTask, guiRenderer, move(drawable))
{
	logClient.setPrefix("[EditMode]");
	mainTask.getWindow()->showCursor(false);
}

void App::EditMode::frameUpdate(ControllerStateMachine & stateMachine)
{
	BaseController::frameUpdate(stateMachine);
	//std::cout << "EditMode::frameUpdate(ControllerStateMachine &) called!" << std::endl;
	Input* input = mainTask->getWindow()->getInputDevice();

	// Switch to camera mode?
	if (input->isPressed(Input::KEY_C)) {
		stateMachine.setCurrentController(std::make_unique<CameraMode>(*mainTask, *guiRenderer, move(m_drawable)));
	}
}

App::CameraMode::CameraMode(PBR_Deferred_MainLoopTask & mainTask, ImGUI_Impl& guiRenderer, std::unique_ptr<nex::engine::gui::Drawable> drawable) :
	BaseController(mainTask, guiRenderer, move(drawable))
{
	logClient.setPrefix("[CameraMode]");
	mainTask.getWindow()->showCursor(true);
}

void App::CameraMode::frameUpdate(ControllerStateMachine & stateMachine)
{
	BaseController::frameUpdate(stateMachine);

	//std::cout << "CameraMode::frameUpdate(ControllerStateMachine &) called!" << std::endl;
	Input* input = mainTask->getWindow()->getInputDevice();
	float frameTime = mainTask->getTimer()->getLastUpdateTimeDifference();

	// update camera
	updateCamera(input, frameTime);

	// Switch to gui mode?
	if (input->isPressed(Input::KEY_C)) {
		stateMachine.setCurrentController(std::make_unique<EditMode>(*mainTask, *guiRenderer, move(m_drawable)));
	}
}

void App::CameraMode::updateCamera(Input * input, float deltaTime)
{
	Camera* camera = mainTask->getCamera();
	Window* window = mainTask->getWindow();

	camera->update(input, deltaTime);
	window->setCursorPosition(window->getWidth() / 2, window->getHeight() / 2);
}