#include <nex/platform/Window.hpp>
#include <gui/Controller.hpp>
#include <PBR_Deferred_Renderer.hpp>
#include <nex/platform/Input.hpp>
#include "nex/gui/ControllerStateMachine.hpp"
#include <nex/gui/Gizmo.hpp>
#include <nex/gui/Picker.hpp>
#include <functional>
#include <imgui/imgui_internal.h>
#include <nex/gui/vob/VobView.hpp>
#include <nex/GI/PbrProbe.hpp>
#include <gui/FontManager.hpp>


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

	/*if (m_input->isPressed(Input::KEY_Y))
	{
		m_mainTask->setShowDepthMap(!m_mainTask->getShowDepthMap());
	}*/


	// Context refresh Does not work right now!
	/*if (m_input->isPressed(Input::KEY_B)) {
		if (m_window->isInFullscreenMode()) {
			m_window->setWindowed();
		}
		else {
			m_window->setFullscreen();
		}

		LOG(m_logger, Debug) << "toggle";
	}*/
}

void nex::gui::BaseController::activateSelf()
{
}

void nex::gui::BaseController::deactivateSelf()
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


nex::gui::EditMode::GizmoGUI::GizmoGUI(Gizmo* gizmo, Input* input) : mGizmo(gizmo), mInput(input)
{
}

void nex::gui::EditMode::GizmoGUI::drawSelf()
{
	if (!mGizmo->isVisible()) return;


	ImGuiID id = GImGui->CurrentWindow->GetID("Gizmo-Selection-Mode");
	if (mInput->isPressed(Input::KEY_TAB) && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
		ImGui::OpenPopupEx(id);
	if (ImGui::BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings))
	{
		if (ImGui::Button("Rotate"))
		{
			mGizmo->setMode(Gizmo::Mode::ROTATE);
			ImGui::CloseCurrentPopup();
		}

		if (ImGui::Button("Scale"))
		{
			mGizmo->setMode(Gizmo::Mode::SCALE);
			ImGui::CloseCurrentPopup();
		}

		if (ImGui::Button("Translate"))
		{
			mGizmo->setMode(Gizmo::Mode::TRANSLATE);
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

nex::gui::EditMode::EditMode(nex::Window* window, nex::Input* input, PerspectiveCamera* camera, Scene* scene, SceneGUI* sceneGUI,
	FontManager* fontManager) :
	Controller(input),
	mWindow(window),
	mCamera(camera),
	mScene(scene),
	mSceneGUI(sceneGUI),
	mGizmo(std::make_unique<Gizmo>()),
	mGizmoGUI(mGizmo.get(), mInput),
	mFontManager(fontManager)
{
}

nex::gui::EditMode::~EditMode() = default;

void nex::gui::EditMode::updateAlways()
{
	if (mGizmo->isVisible())
	{
		auto* picker = mSceneGUI->getPicker();

		auto* activeVob = picker->getPicked();
		if (activeVob == nullptr) {
			picker->deselect(*mScene);
			mGizmo->hide();
		}
		else {
			mGizmo->update(*mCamera, activeVob);
			picker->updateBoundingBoxTrafo();
		}
		
	}
}

void nex::gui::EditMode::frameUpdateSelf(float frameTime)
{
	const auto& mouseData = mInput->getFrameMouseOffset();
	const auto activateButton = Input::Button::LeftMouseButton;
	const auto deactivateButton = Input::Button::RightMouseButton;
	glm::ivec2 screenDimension(static_cast<int>(mWindow->getFrameBufferWidth()),
								static_cast<int>(mWindow->getFrameBufferHeight()));


	auto* picker = mSceneGUI->getPicker();

	if (mInput->isPressed(activateButton))
	{

		const glm::ivec2 position(mouseData.xAbsolute, mouseData.yAbsolute);
		const auto ray = mCamera->calcScreenRay(position, screenDimension);
		activate(ray);
	}
	else if (mInput->isDown(activateButton))
	{
		const glm::ivec2 position(mouseData.xAbsolute, mouseData.yAbsolute);
		const auto ray = mCamera->calcScreenRay(position, screenDimension);
		mGizmo->transform(ray, *mCamera, mouseData);
		picker->updateBoundingBoxTrafo();
	}
	else if (mInput->isReleased(activateButton))
	{
		mGizmo->deactivate();
	}
	else if (mInput->isPressed(deactivateButton))
	{
		picker->deselect(*mScene);
		mGizmo->hide();
	}



	if (mInput->isDown(Input::KEY_LEFT_CONTROL)) {

		auto scrollY = mInput->getFrameScrollOffsetY();

		//LOG(Logger("EditMode"), Info) << "Scroll Y: " << scrollY;
		auto scale = mFontManager->getGlobalFontScale();
		mFontManager->setGlobalFontScale(scale + scrollY * 0.1f);
	}

	if (mInput->isDown(Input::KEY_LEFT_CONTROL) && mInput->isPressed(Input::KEY_BACKSPACE)) {

		auto* mainMenuBar = mSceneGUI->getMainMenuBar();
		mainMenuBar->setVisible(!mainMenuBar->isVisible(), false);
	}
}




void nex::gui::EditMode::activateSelf()
{
	mWindow->showCursor(CursorState::Normal);
	mSceneGUI->addChild(&mGizmoGUI);

	auto* picker = mSceneGUI->getPicker();

	auto callback = std::bind(&EditMode::updateVobView, this, std::placeholders::_1);
	mPickedChangeCallbackHandle = picker->addPickedChangeCallback(callback);
}

void nex::gui::EditMode::deactivateSelf()
{
	auto& childs = mSceneGUI->getChilds();
	childs.erase(std::remove(childs.begin(), childs.end(), nex::flexible_ptr<Drawable>(&mGizmoGUI, false)), childs.end());

	auto* picker = mSceneGUI->getPicker();
	picker->removePickedChangeCallback(mPickedChangeCallbackHandle);
	mPickedChangeCallbackHandle = {};
}

bool nex::gui::EditMode::isNotInterruptibleActionActiveSelf() const
{
	return false;
}

void nex::gui::EditMode::activate(const Ray& ray)
{
	bool picked = false;
	auto& scene = *mScene;
	const bool isVisible = mGizmo->isVisible();


	auto* picker = mSceneGUI->getPicker();

	if (isVisible)
	{
		const auto isHovering = mGizmo->isHovering(ray, *mCamera);
		if (!isHovering)
		{
			picked = picker->pick(scene, ray) != nullptr;
		}

		if (!isHovering && !picked)
		{
			mGizmo->hide();
		}

		if (isHovering)
		{
			mGizmo->activate(ray, *mCamera);
		}

	}
	else
	{
		picker->pick(scene, ray);
	}
}

void nex::gui::EditMode::updateVobView(Vob* pickedVob)
{
	auto* vobEditor = mSceneGUI->getVobEditor();
	vobEditor->updateVobView(pickedVob);

	const bool isVisible = mGizmo->isVisible();

	if (pickedVob && !isVisible)
	{
		mGizmo->show(mScene);
	}
}

nex::gui::CameraMode::CameraMode(nex::Window* window, nex::Input* input, Camera* camera) :
	Controller(input),
	mWindow(window), mCamera(camera)
{
	// We don't want to be interrupted by ui
	mAllowInputForUi = false;
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

void nex::gui::CameraMode::deactivateSelf()
{
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

nex::gui::EngineController::EngineController(nex::Window* window, 
	Input* input, 
	PBR_Deferred_Renderer* mainTask, 
	PerspectiveCamera* camera, 
	Picker* picker,
	Scene* scene,
	ImGUI_Impl* guiImpl,
	FontManager* fontManager) :
ControllerStateMachine(input,nullptr),
mBaseController(window, input, mainTask),
mSceneGUI(window, picker, scene, camera, std::bind(&BaseController::handleExitEvent, &mBaseController)),
mEditMode(window, input, camera, scene, &mSceneGUI, fontManager),
mCameraMode(window, input, camera),
mGuiImpl(guiImpl),
mCamera(camera)
{
	setActiveController(&mEditMode);
	addChild(&mBaseController);
	setDrawable(&mSceneGUI);
}

void nex::gui::EngineController::frameUpdateSelf(float frameTime)
{
	mEditMode.updateAlways();

	// Switch mode?
	if (mInput->isPressed(Input::KEY_F1)) {
				
		if (mActiveController == &mEditMode)
		{
			setActiveController(&mCameraMode);
		} else
		{
			setActiveController(&mEditMode);
		}
	}

	if (mGuiImpl->isActive() && !mActiveController->isNotInterruptibleActionActive())
		return;

	ControllerStateMachine::frameUpdateSelf(frameTime);
}

void nex::gui::EngineController::onWindowsResize(unsigned width, unsigned height)
{
	mSceneGUI.onCanvasResize(width, height);
}

nex::gui::SceneGUI* nex::gui::EngineController::getSceneGUI()
{
	return &mSceneGUI;
}

nex::gui::EditMode* nex::gui::EngineController::getEditMode()
{
	return &mEditMode;
}

nex::gui::EngineController::~EngineController() = default;