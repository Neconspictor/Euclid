#include <nex/Window.hpp>
#include <gui/Controller.hpp>
#include <pbr_deferred/PBR_Deferred_Renderer.hpp>
#include <nex/Input.hpp>
#include "nex/gui/ControllerStateMachine.hpp"


nex::gui::BaseController::BaseController(nex::Window* window, Input* input, PBR_Deferred_Renderer* mainTask, ImGUI_Impl* guiRenderer, std::unique_ptr<nex::gui::Drawable> drawable) :
	Controller(std::move(drawable)),
	m_window(window),
	m_input(input),
	guiRenderer(guiRenderer),
	m_mainTask(mainTask),
	m_logger("BaseController")
{
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

void nex::gui::BaseController::frameUpdate(ControllerStateMachine & stateMachine, float frameTime)
{
	using namespace nex;

	if (m_input->isPressed(Input::KEY_ESCAPE))
	{
		handleExitEvent();
	}

	if (m_input->isPressed(Input::KEY_Y))
	{
		m_mainTask->setShowDepthMap(!m_mainTask->getShowDepthMap());
	}


	// Context refresh Does not work right now!
	if (m_input->isPressed(Input::KEY_B)) {
		if (m_window->isInFullscreenMode()) {
			m_window->setWindowed();
		}
		else {
			m_window->setFullscreen();
		}

		LOG(m_logger, Debug) << "toggle";
	}
}

void nex::gui::BaseController::init()
{
}

void nex::gui::BaseController::handleExitEvent()
{
	m_window->close();
}


nex::gui::EditMode::EditMode(nex::Window* window, nex::Input* input, PBR_Deferred_Renderer* mainTask,
	Camera* camera,
	ImGUI_Impl* guiRenderer, 
	std::unique_ptr<nex::gui::Drawable> drawable) :
	BaseController(window, input, mainTask, guiRenderer, move(drawable)),
	m_camera(camera)
{
	m_logger.setPrefix("EditMode");
	m_window->showCursor(CursorState::Normal);
}

void nex::gui::EditMode::frameUpdate(ControllerStateMachine & stateMachine, float frameTime)
{
	BaseController::frameUpdate(stateMachine, frameTime);
	//std::cout << "EditMode::frameUpdate(ControllerStateMachine &) called!" << std::endl;

	// Switch to camera mode?
	if (m_input->isPressed(Input::KEY_C)) {
		stateMachine.setCurrentController(std::make_unique<CameraMode>(m_window, m_input, m_mainTask, m_camera, guiRenderer, move(m_drawable)));
	}
}

nex::gui::CameraMode::CameraMode(nex::Window* window,
	nex::Input* input,
	PBR_Deferred_Renderer* mainTask, 
	Camera* camera, ImGUI_Impl* guiRenderer, 
	std::unique_ptr<nex::gui::Drawable> drawable) :
	BaseController(window, input, mainTask, guiRenderer, std::move(drawable)),
m_window(window), m_camera(camera)
{
	m_logger.setPrefix("CameraMode");
	m_window->showCursor(CursorState::Disabled);
}

void nex::gui::CameraMode::frameUpdate(ControllerStateMachine & stateMachine, float frameTime)
{
	BaseController::frameUpdate(stateMachine, frameTime);

	//std::cout << "CameraMode::frameUpdate(ControllerStateMachine &) called!" << std::endl;
	//float frameTime = mainTask->getTimer()->getLastUpdateTimeDifference();

	// update camera
	updateCamera(m_input, frameTime);

	// Switch to gui mode?
	if (m_input->isPressed(Input::KEY_C)) {
		stateMachine.setCurrentController(std::make_unique<EditMode>(m_window, m_input, m_mainTask, m_camera, guiRenderer, move(m_drawable)));
	}
}

void nex::gui::CameraMode::updateCamera(Input * input, float deltaTime)
{
	m_camera->update(input, deltaTime);
	m_window->setCursorPosition(m_window->getFrameBufferWidth() / 2, m_window->getFrameBufferHeight() / 2);
}