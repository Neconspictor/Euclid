#include <nex/Window.hpp>
#include <gui/Controller.hpp>
#include <nex/Input.hpp>
#include <nex/gui/ControllerStateMachine.hpp>


nex::gui::BaseController::BaseController(nex::Window* window, Input* input, Renderer* mainTask, ImGUI_Impl* guiRenderer, std::unique_ptr<nex::gui::Drawable> drawable) :
	Controller(std::move(drawable)),
	m_window(window),
	m_input(input),
	guiRenderer(guiRenderer),
	m_mainTask(mainTask),
	m_logger("BaseController")
{
}

void nex::gui::BaseController::frameUpdate(ControllerStateMachine & stateMachine, float frameTime)
{
	using namespace nex;

	if (m_input->isPressed(Input::KEY_ESCAPE))
	{
		handleExitEvent();
	}

	if (m_input->isPressed(Input::KEY_Y))
	{
		//m_mainTask->setShowDepthMap(!m_mainTask->getShowDepthMap());
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


nex::gui::EditMode::EditMode(nex::Window* window, nex::Input* input, Renderer* mainTask,
	Camera* camera,
	ImGUI_Impl* guiRenderer, 
	std::unique_ptr<nex::gui::Drawable> drawable) :
	BaseController(window, input, mainTask, guiRenderer, move(drawable)),
	m_camera(camera)
{
	m_logger.setPrefix("EditMode");
	m_window->showCursor(true);
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
	Renderer* mainTask, 
	Camera* camera, ImGUI_Impl* guiRenderer, 
	std::unique_ptr<nex::gui::Drawable> drawable) :
	BaseController(window, input, mainTask, guiRenderer, std::move(drawable)),
m_window(window), m_camera(camera)
{
	m_logger.setPrefix("CameraMode");
	m_window->showCursor(false);
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