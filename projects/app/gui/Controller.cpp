#include <nex/Window.hpp>
#include <gui/Controller.hpp>
#include <pbr_deferred/PBR_Deferred_Renderer.hpp>
#include <nex/Input.hpp>
#include "nex/gui/ControllerStateMachine.hpp"
#include <nex/gui/Gizmo.hpp>
#include <nex/gui/Picker.hpp>
#include <functional>


nex::gui::BaseController::BaseController(nex::Window* window, Input* input, PBR_Deferred_Renderer* mainTask) :
	Controller(input),
	m_window(window),
	m_input(input),
	m_mainTask(mainTask),
	m_logger("BaseController")
{
}

void nex::gui::BaseController::frameUpdateSelf(float frameTime)
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

void nex::gui::BaseController::activateSelf()
{
}

bool nex::gui::BaseController::isNotInterruptibleActionActiveSelf() const
{
	return false;
}

void nex::gui::BaseController::handleExitEvent()
{
	m_window->close();
}


nex::gui::EditMode::EditMode(nex::Window* window, nex::Input* input, Camera* camera) :
	Controller(input),
	mWindow(window),
	mCamera(camera)
{
}

nex::gui::EditMode::~EditMode() = default;

void nex::gui::EditMode::frameUpdateSelf(float frameTime)
{
}

void nex::gui::EditMode::activateSelf()
{
	mWindow->showCursor(CursorState::Normal);
}

bool nex::gui::EditMode::isNotInterruptibleActionActiveSelf() const
{
	return false;
}

nex::gui::CameraMode::CameraMode(nex::Window* window, nex::Input* input, Camera* camera) :
	Controller(input),
	mWindow(window), mCamera(camera)
{
}

nex::gui::CameraMode::~CameraMode() = default;

void nex::gui::CameraMode::frameUpdateSelf(float frameTime)
{
	updateCamera(frameTime);
}

void nex::gui::CameraMode::activateSelf()
{
	mWindow->showCursor(CursorState::Disabled);
}

bool nex::gui::CameraMode::isNotInterruptibleActionActiveSelf() const
{
	// During this mode we don't want to get interrupted!
	return true;
}

void nex::gui::CameraMode::updateCamera(float deltaTime)
{
	mCamera->frameUpdate(mInput, deltaTime);
	mWindow->setCursorPosition(mWindow->getFrameBufferWidth() / 2, mWindow->getFrameBufferHeight() / 2);
}

nex::gui::EngineController::EngineController(nex::Window* window, Input* input, PBR_Deferred_Renderer* mainTask, Camera* camera) : 
ControllerStateMachine(input,nullptr),
mBaseController(window, input, mainTask),
mEditMode(window, input, camera),
mCameraMode(window, input, camera),
mSceneGUI(std::bind(&BaseController::handleExitEvent, &mBaseController))
{
	setActiveController(&mEditMode);
	addChild(&mBaseController);
	setDrawable(&mSceneGUI);
}

void nex::gui::EngineController::frameUpdateSelf(float frameTime)
{
	// Switch mode?
	if (mInput->isPressed(Input::KEY_C)) {
		if (getActiveController() == &mEditMode)
		{
			setActiveController(&mCameraMode);
		} else
		{
			setActiveController(&mEditMode);
		}
	}

	ControllerStateMachine::frameUpdateSelf(frameTime);
}

nex::gui::SceneGUI* nex::gui::EngineController::getSceneGUI()
{
	return &mSceneGUI;
}

nex::gui::EngineController::~EngineController() = default;
