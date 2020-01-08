#include <nex/gui/vob/VobEditor.hpp>
#include "nex/gui/ImGUI_Extension.hpp"
#include "nex/gui/Picker.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "nex/pbr/PbrProbe.hpp"
#include "nex/texture/TextureManager.hpp"
#include <nex/platform/Window.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/gui/vob/VobView.hpp>
#include <nex\water\Ocean.hpp>
#include <nex/gui/vob/VobViewMapper.hpp>

namespace nex::gui
{
	VobEditor::VobEditor(nex::Window* window, Picker* picker, Scene* scene, Camera* camera, float splitterPosition) :
		mWindow(window),
		mScene(nullptr),
		mLastPickedVob(nullptr),
		mPicker(picker),
		mVobView(nullptr),
		mCamera(camera),
		mSplitterPosition(0.0f),
		mLeftMinSize(ImVec2(0,0)),
		mInit(true),
		mSceneView(picker, scene)
		//mTransparentView({}, ImVec2(256, 256))
	{
	}

	VobEditor::~VobEditor() = default;

	void VobEditor::updateVobView(Vob* vob)
	{
		mVobView = VobViewMapper::getViewByVob(vob);
	}

	void VobEditor::setVisible(bool visible)
	{
		Drawable::setVisible(visible);
		if (visible) {
			mLeftMinSize = ImVec2(0,0);
			mRightContentSize = ImVec2(0,0);
		}
	}

	void nex::gui::VobEditor::drawSelf()
	{
		ImGuiContext& g = *GImGui;
		float h = ImGui::GetWindowHeight();
		float sz2 = ImGui::GetWindowContentRegionWidth();

		//mSplitterPosition = mLeftContentSize.x;

		mSplitterPosition = mLeftMinSize.x;
		Splitter("##Splitter", true, 8.0f, &mSplitterPosition, &sz2, 8, 8, h);

		/*static int counter = 0;
		if (counter == 300) {
			counter = -1;
			mSplitterPosition = mSplitterPosition - 50;
		}
		++counter;*/

		if (ImGui::BeginChild("left", ImVec2(mSplitterPosition, mInitialHeight), true, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar)) { // ImGuiWindowFlags_HorizontalScrollbar

			ImGui::BeginGroup();
				mSceneView.drawGUI();
			ImGui::EndGroup();

			auto* window = ImGui::GetCurrentWindow();
			mLeftContentSize = ImGui::CalcWindowExpectedSize(window);
			if (window->ScrollbarX || window->ScrollbarY)
				mLeftMinSize = mLeftContentSize;
		}

		ImGui::EndChild();


		ImGui::SameLine();

		if (ImGui::BeginChild("right", ImVec2(mRightContentSize.x, mInitialHeight), true, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar)) { //ImGuiWindowFlags_HorizontalScrollbar

			ImGui::BeginGroup();
				drawRightSideContent();
			ImGui::EndGroup();

			auto* window = ImGui::GetCurrentWindow();
			mRightContentSize = ImGui::CalcWindowExpectedSize(window);
		}

		ImGui::EndChild();


		auto windowSize = ImGui::GetWindowSize();

		if (mInit) {
			mInitialHeight = max(mLeftContentSize.y, mRightContentSize.y);

			ImGuiContext& g = *GImGui;
			//ImGui::SetWindowSize(ImVec2(mLeftContentSize.x + mRightContentSize.x, mInitialHeight));
			//mInit = false;
		}
	}


	void VobEditor::drawRightSideContent()
	{
		Vob* vob = mPicker->getPicked();
		bool doOneTimeChanges = vob != mLastPickedVob;
		mLastPickedVob = vob;

		if (!vob) {
			ImGui::Text("No vob selected.");
		}
		else if (!mVobView) {
			ImGui::Text("No view found.");
		}
		else {
			mVobView->draw(vob, mScene, mPicker, mCamera, doOneTimeChanges);
		}
	}
}