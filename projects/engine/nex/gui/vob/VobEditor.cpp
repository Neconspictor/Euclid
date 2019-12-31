#include <nex/gui/vob/VobEditor.hpp>
#include <imgui/imgui.h>
#include "nex/gui/Util.hpp"
#include "nex/gui/Picker.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "nex/pbr/PbrProbe.hpp"
#include "nex/texture/TextureManager.hpp"
#include <nfd/nfd.h>
#include <nex/platform/Window.hpp>
#include <nex/resource/ResourceLoader.hpp>
#include <nex/gui/FileDialog.hpp>
#include <nex/mesh/MeshManager.hpp>
#include <boxer/boxer.h>
#include <nex/gui/vob/VobView.hpp>

#ifndef IMGUI_DEFINE_MATH_OPERATORS 
#define IMGUI_DEFINE_MATH_OPERATORS 
#endif

#include <imgui/imgui_internal.h>

namespace nex::gui
{
	VobEditor::VobEditor(nex::Window* window, Picker* picker, Camera* camera, float splitterPosition) :
		mWindow(window),
		mScene(nullptr),
		mLastPickedVob(nullptr),
		mPicker(picker),
		mVobView(nullptr),
		mCamera(camera),
		mSplitterPosition(splitterPosition),
		mInit(true)
		//mTransparentView({}, ImVec2(256, 256))
	{	
	}

	VobEditor::~VobEditor() = default;

	void VobEditor::setScene(nex::Scene * scene)
	{
		mScene = scene;
	}

	void VobEditor::setVobView(VobView* view)
	{
		mVobView = view;
	}


	// Calculates rotation matrix given euler angles.
	glm::mat3 eulerAnglesToRotationMatrix(const glm::vec3& theta)
	{
		// Calculate rotation about x axis
		glm::mat3 R_x = {
			1, 0, 0,
			0, cos(theta[0]), -sin(theta[0]),
			0, sin(theta[0]), cos(theta[0])
		};

		// Calculate rotation about y axis
		glm::mat3 R_y = {
			cos(theta[1]), 0, sin(theta[1]),
			0, 1, 0,
			-sin(theta[1]), 0, cos(theta[1])
		};

		// Calculate rotation about z axis
		glm::mat3 R_z = {
			cos(theta[2]), -sin(theta[2]), 0,
			sin(theta[2]), cos(theta[2]), 0,
			0, 0, 1 };


		// Combined rotation matrix
		return transpose(R_z) * transpose(R_y) * transpose(R_x);
	}

	// Calculates rotation matrix to euler angles
// The result is the same as MATLAB except the order
// of the euler angles ( x and z are swapped ).
	glm::vec3 rotationMatrixToEulerAngles(const glm::mat4 &R)
	{
		float sy = sqrt(R[0][0] * R[0][0] + R[0][1] * R[0][1]);

		bool singular = sy < 1e-6; // If

		float x, y, z;
		if (!singular)
		{
			x = atan2(R[1][2], R[2][2]);
			y = atan2(-R[0][2], sy);
			z = atan2(R[0][1], R[0][0]);
		}
		else
		{
			x = atan2(-R[2][1], R[1][1]);
			y = atan2(-R[0][2], sy);
			z = 0;
		}
		return { x, y, z };
	}


	bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f)
	{
		using namespace ImGui;
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		ImGuiID id = window->GetID("##Splitter");
		ImRect bb;
		bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
		bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
		return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
	}

	void nex::gui::VobEditor::drawSelf()
	{

		float h = ImGui::GetWindowHeight();
		float sz2 = ImGui::GetWindowContentRegionWidth();

		Splitter(true, 8.0f, &mSplitterPosition, &sz2, 8, 8, h);
		

		auto leftContentSize = ImVec2(0,0);
		auto rightContentSize = ImVec2(0, 0);

		
		if (ImGui::BeginChild("left", ImVec2(mSplitterPosition, 0), true, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar)) { // ImGuiWindowFlags_HorizontalScrollbar
			

			ImGui::BeginGroup();
			ImGuiContext& g = *GImGui;
			ImGuiWindow* window = g.CurrentWindow;
			const auto& cp = window->DC.CursorPos;
			auto textSize = ImGui::CalcTextSize("Scene");
			
			auto min = ImVec2(cp.x - g.Style.FramePadding.x, cp.y + textSize.y + g.Style.FramePadding.y);
			auto max = ImVec2(cp.x + textSize.x + g.Style.FramePadding.x, cp.y - g.Style.FramePadding.y);

			

			window->DrawList->AddRectFilled(min, max, ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)));
			ImGui::Text("Scene");
			

			ImGui::TreePush("SceneNodes");
			{
				auto lock = mScene->acquireLock();

				Vob* selectedVob = nullptr;

				for (auto* vob : (mScene->getActiveRootsUnsafe())) {

					if (!vob->getSelectable()) continue;



					auto* currentSelected = drawVobHierarchy(vob);
					if (!selectedVob)selectedVob = currentSelected;
				}

				if (selectedVob) {
					mPicker->select(*mScene, selectedVob);
				}

				
			}
			
			ImGui::TreePop();

			ImGui::EndGroup();


			leftContentSize = ImGui::GetItemRectSize();
			leftContentSize.x += g.Style.WindowPadding.x + g.Style.FramePadding.x * 2;
			leftContentSize.y += g.Style.WindowMinSize.y + g.Style.FramePadding.y*2 + g.Style.WindowPadding.y*2;
		}
		
		

		ImGui::EndChild();
		//contentSize += ImGui::GetItemRectMax() - ImGui::GetItemRectMin();


		ImGui::SameLine();

		if (ImGui::BeginChild("right", ImVec2(0,0), true, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar)) { //ImGuiWindowFlags_HorizontalScrollbar
			
			ImGui::BeginGroup();

			Vob* vob = mPicker->getPicked();
			bool doOneTimeChanges = vob != mLastPickedVob;
			mLastPickedVob = vob;

			if (!vob) {
				ImGui::Text("No vob selected.");
			} else if (!mVobView) {
				ImGui::Text("No view found.");
			}
			else {
				mVobView->draw(vob, mScene, mPicker, mCamera, doOneTimeChanges);
			}

			ImGui::EndGroup();
			ImGuiContext& g = *GImGui;
			rightContentSize = ImGui::GetItemRectSize();
			rightContentSize.x += g.Style.WindowMinSize.x + g.Style.WindowPadding.x*2 + g.Style.FramePadding.x * 2;
			rightContentSize.y += g.Style.WindowMinSize.y + g.Style.FramePadding.y * 2 + g.Style.WindowPadding.y * 2;
		}

		ImGui::EndChild();


			

		if (mInit) {
			mInitialHeight = max(leftContentSize.y, rightContentSize.y);
			mSplitterPosition = leftContentSize.x;
			ImGui::SetWindowSize(ImVec2(leftContentSize.x + rightContentSize.x, mInitialHeight));
		}

		
		

		

		//nex::gui::Separator(2.0f);

		
	}
	Vob* VobEditor::drawVobHierarchy(Vob* vob)
	{
		nex::gui::ID vobID((int)vob);

		auto& children = vob->getChildren();

		const char* name = vob->getName().c_str();

		if (children.size() > 0) {

			Vob* selectedVob = nullptr;
			
			if (ImGui::TreeNodeEx(name)) {
				for (auto* child : children) {
					auto* selectedChild = drawVobHierarchy(child);
					if (selectedVob == nullptr) selectedVob = selectedChild;
				}

				ImGui::TreePop();
			}

			if (ImGui::IsItemClicked() && !selectedVob) {
				selectedVob = vob;
			}

			return selectedVob;
		}

		if (ImGui::ButtonEx(vob->getName().c_str(), ImVec2(0,0), ImGuiButtonFlags_PressedOnClick)) {
				return vob;
		}


		return nullptr;
	}
}