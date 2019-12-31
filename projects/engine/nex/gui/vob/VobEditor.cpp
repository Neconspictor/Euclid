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
	VobEditor::VobEditor(nex::Window* window, Picker* picker) :
		mWindow(window),
		mScene(nullptr),
		mLastPickedVob(nullptr),
		mPicker(picker),
		mVobView(nullptr)
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

		float h = 200;
		static float sz1 = 300;
		static float sz2 = 300;
		Splitter(true, 8.0f, &sz1, &sz2, 8, 8, h);
		
		
		if (ImGui::BeginChild("left", ImVec2(sz1, h), true)) {
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

				for (auto* vob : (mScene->getActiveVobsUnsafe())) {

					if (!vob->getSelectable()) continue;

					if (ImGui::Button(vob->getName().c_str())) {
						selectedVob = vob;
					}
				}

				if (selectedVob) {
					mPicker->select(*mScene, selectedVob);
				}
			}
			
			ImGui::TreePop();
		}
		ImGui::EndChild();
		


		ImGui::SameLine();

		if (ImGui::BeginChild("right", ImVec2(sz2, h), true)) {
			Vob* vob = mPicker->getPicked();
			bool doOneTimeChanges = vob != mLastPickedVob;
			mLastPickedVob = vob;

			if (!vob) {
				ImGui::Text("No scene node selected.");
			} else if (!mVobView) {
				ImGui::Text("No view found.");
			}
			else {
				mVobView->draw(vob, mScene, mPicker, doOneTimeChanges);
			}
		}
		ImGui::EndChild();
		
		

		

		//nex::gui::Separator(2.0f);

		
	}
}