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

	void VobEditor::setScene(nex::Scene* scene)
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
	glm::vec3 rotationMatrixToEulerAngles(const glm::mat4& R)
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



		if (ImGui::BeginChild("left", ImVec2(mSplitterPosition, mInitialHeight), true, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar)) { // ImGuiWindowFlags_HorizontalScrollbar

			ImGui::BeginGroup();
				ImGuiContext& g = *GImGui;
				mLeftContentPadding = g.Style.WindowPadding + g.Style.FramePadding;
				drawScene();
			ImGui::EndGroup();


			mLeftContentSize = ImGui::GetItemRectSize();
			mLeftContentSize.x += g.Style.WindowPadding.x + g.Style.FramePadding.x + g.Style.ScrollbarSize + g.Style.WindowBorderSize * 2;
			mLeftContentSize.y += 2 * (g.Style.WindowPadding.y + g.Style.FramePadding.y + g.Style.ScrollbarSize + g.Style.WindowBorderSize);
		}

		ImGui::EndChild();


		ImGui::SameLine();

		if (ImGui::BeginChild("right", ImVec2(mRightContentSize.x, mInitialHeight), true, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar)) { //ImGuiWindowFlags_HorizontalScrollbar

			ImGui::BeginGroup();

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

			ImGui::EndGroup();
			ImGuiContext& g = *GImGui;
			mRightContentSize = ImGui::GetItemRectSize();
			mRightContentSize.x += g.Style.WindowPadding.x + g.Style.FramePadding.x + g.Style.ScrollbarSize + g.Style.WindowBorderSize * 2;
			mRightContentSize.y += 2 * (g.Style.WindowPadding.y + g.Style.FramePadding.y + g.Style.ScrollbarSize + g.Style.WindowBorderSize);

		}

		ImGui::EndChild();

		auto rightSize = ImGui::GetItemRectSize();
		auto rightSizeMin = ImGui::GetItemRectMin();
		auto rightSizeMax = ImGui::GetItemRectMax();


		auto windowSize = ImGui::GetWindowSize();

		if (mInit) {



			mInitialHeight = max(mLeftContentSize.y, mRightContentSize.y);

			ImGuiContext& g = *GImGui;

			mSplitterPosition = mLeftContentSize.x - mLeftContentPadding.x;
			//ImGui::SetWindowSize(ImVec2(mLeftContentSize.x + mRightContentSize.x, mInitialHeight));
			//mInit = false;
		}






		//nex::gui::Separator(2.0f);


	}
	Vob* VobEditor::drawVobHierarchy(Vob* vob)
	{

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems) return nullptr;

		nex::gui::ID vobID((int)vob);

		auto& children = vob->getChildren();

		const char* name = vob->getName().c_str();

		Vob* selectedVob = nullptr;
		ImGui::Bullet();

		if (children.size() > 0) {

			auto open = nex::gui::TreeNodeExCustomShape(name, drawCustomVobHeader,
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_OpenOnArrow); //ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_OpenOnArrow);
			auto toggled = ImGui::IsItemToggledOpen();
			auto released = ImGui::IsMouseReleased(0);
			auto hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_None);
			drawDragDrop(vob);

			if (open) {
				for (auto* child : children) {
					auto* selectedChild = drawVobHierarchy(child);
					if (selectedVob == nullptr) selectedVob = selectedChild;
				}

				ImGui::TreePop();
			}

			if (released && hovered && !selectedVob && !toggled) {
				selectedVob = vob;
			}

		}
		else
		{
			if (ImGui::ButtonEx(vob->getName().c_str(), ImVec2(0, 0))) { //ImGuiButtonFlags_PressedOnClick
				selectedVob = vob;
			}
			drawDragDrop(vob);

		}



		return selectedVob;
	}

	void VobEditor::drawScene()
	{
		if (drawSceneRoot()) {
			
			ImGuiStyle& style = ImGui::GetStyle();
			StyleColorPush buttonBackground(ImGuiCol_Button, style.Colors[ImGuiCol_WindowBg]);
			StyleColorPush headerHovered(ImGuiCol_HeaderHovered, style.Colors[ImGuiCol_ButtonHovered]);
			StyleColorPush header(ImGuiCol_Header, style.Colors[ImGuiCol_Button]);
			StyleColorPush headerActive(ImGuiCol_HeaderActive, style.Colors[ImGuiCol_ButtonActive]);
			StyleColorPush navHighlight(ImGuiCol_NavHighlight, style.Colors[ImGuiCol_WindowBg]);

			drawDragDropRearrange(0);

			auto lock = mScene->acquireLock();

			Vob* selectedVob = nullptr;


			auto roots = mScene->getActiveRootsUnsafe();
			for (int i = 0; i < roots.size(); ++i) {

				auto* vob = roots[i];
				if (!vob->getSelectable()) continue;

				auto* currentSelected = drawVobHierarchy(vob);
				if (!selectedVob)selectedVob = currentSelected;

				drawDragDropRearrange(i + 1);
			}

			if (selectedVob) {
				mPicker->select(*mScene, selectedVob);
			}
			endSceneRoot();
		}
	}

	bool VobEditor::drawSceneRoot()
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;
		const auto& cp = window->DC.CursorPos;
		const char* text = "Scene";
		auto textSize = ImGui::CalcTextSize(text);

		auto min = ImVec2(cp.x - g.Style.FramePadding.x, cp.y + textSize.y + g.Style.FramePadding.y);
		auto max = ImVec2(cp.x + textSize.x + g.Style.FramePadding.x, cp.y - g.Style.FramePadding.y);

		//window->DrawList->AddRectFilled(min, max, ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)));
		bool open = true;

		if (false) {
			ImGui::Text(text);
			drawDragDropRoot();
			ImGui::TreePush(text);
		}
		else {



			//drawCustom();
			open = nex::gui::TreeNodeExCustomShape(text, drawCustomRootHeader, 
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen);
			drawDragDropRoot();
			
		}
		

		return open;
	}

	void VobEditor::endSceneRoot()
	{
		ImGui::TreePop();
	}

	void VobEditor::drawDragDrop(Vob* vob)
	{
		if (ImGui::BeginDragDropSource()) {

			if (auto* payload = ImGui::GetDragDropPayload()) {
				if (strncmp(payload->DataType, "vob", 3) != 0) {
					ImGui::SetDragDropPayload("vob", &vob, sizeof(Vob**));
				}
			}

			ImGui::Text(vob->getName().c_str());

			ImGui::EndDragDropSource();
		}


		if (ImGui::BeginDragDropTarget()) {
			auto* payload = ImGui::AcceptDragDropPayload("vob");

			if (payload) {
				auto* newChild = *(Vob**)payload->Data;
				if (auto* oldParent = newChild->getParent()) {
					oldParent->removeChild(newChild);
				}

				RenderEngine::getCommandQueue()->push([=]() {
					mScene->removeActiveRoot(newChild);
					});

				vob->addChild(newChild);
			}

			ImGui::EndDragDropTarget();
		}
	}
	void VobEditor::drawDragDropRoot()
	{
		if (ImGui::BeginDragDropTarget()) {
			auto* payload = ImGui::AcceptDragDropPayload("vob");

			if (payload) {
				auto* newChild = *(Vob**)payload->Data;
				if (auto* oldParent = newChild->getParent()) {
					oldParent->removeChild(newChild);
				}

				RenderEngine::getCommandQueue()->push([=]() {
					mScene->removeActiveVobUnsafe(newChild, false);
					mScene->addActiveVobUnsafe(newChild, false);
					});
			}

			ImGui::EndDragDropTarget();
		}
	}

	void VobEditor::drawDragDropRearrange(int placerIndex)
	{

		nex::gui::ID vobID(ImGui::GetID("rearrange"));
		auto region = ImGui::GetContentRegionAvail();

		ImGui::InvisibleButton("", ImVec2(region.x, 3));

		if (ImGui::BeginDragDropTarget()) {
			auto* payload = ImGui::AcceptDragDropPayload("vob");

			if (payload) {
				auto* vob = *(Vob**)payload->Data;
				RenderEngine::getCommandQueue()->push([=]() {
					LOG(Logger("VobEditor::drawDragDropRearrange"), Info) << "Rearrange has been executed";

					mScene->acquireLock();
					auto& roots = mScene->getActiveRootsUnsafe();

					if (!vob->isRoot()) {
						//throw_with_trace(std::invalid_argument("dragged vob is no root vob!"));
						roots.insert(roots.begin() + placerIndex, vob);
						vob->getParent()->removeChild(vob);
						return;
					}

					int vobIndex = -1;
					for (int i = 0; i < roots.size(); ++i) {
						if (roots[i] == vob) {
							vobIndex = i;
							break;
						}
					}

					if (vobIndex < 0) {
						throw_with_trace(std::invalid_argument("dragged vob is no root vob!"));
						return;
					}

					if (placerIndex == vobIndex || placerIndex == vobIndex + 1) return;

					auto newVobIndex = (placerIndex == 0) ? 0 : placerIndex - 1;

					roots.erase(roots.begin() + vobIndex);
					roots.insert(roots.begin() + newVobIndex, vob);
					});
			}

			ImGui::EndDragDropTarget();
		}
	}
	void VobEditor::drawCustomRootHeader(ImGuiID id, ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, bool border, float rounding)
	{
		ImGuiContext& g = *GImGui;
		auto& style = g.Style;
		auto* window = ImGui::GetCurrentWindow();
		auto padding = style.FramePadding;
		auto text_size = ImGui::CalcTextSize("Scene");
		const float line_height = ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + g.Style.FramePadding.y * 2), g.FontSize);
		const float frame_height = ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + g.Style.FramePadding.y * 2), text_size.y + padding.y * 2);
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImDrawCornerFlags corners_tl_br = ImDrawCornerFlags_TopLeft | ImDrawCornerFlags_BotRight;
		static float sz = 50.0f;
		static ImVec4 colf = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		const ImU32 col = ImColor(colf);
		const ImU32 blackCol = ImColor(0.0f, 0.0f, 0.0f, 1.0f);
		float th = 1.0f;
		const ImVec2 p = ImGui::GetCursorScreenPos();
		float x = p.x - padding.x, y = p.y;
		auto region = ImGui::GetContentRegionAvail();


		draw_list->Flags |= ImDrawListFlags_AntiAliasedFill;
		//auto& style = ImGui::GetStyle();
		//style.AntiAliasedFill = true;
		//ImGui::GetForegroundDrawList()->
		//	AddRectFilled(ImVec2(x, y), ImVec2(x + region.x + padding.x * 2, y + frame_height), col, 5.0f, corners_tl_br);  // Square with two rounded corners
		draw_list->AddRectFilled(p_min + ImVec2(1, 1), p_max - ImVec2(1, 1), col, 5.0f, corners_tl_br);  // Square with two rounded corners
		draw_list->AddRect(p_min, p_max, blackCol, 5.0f, corners_tl_br, th);  // Square with two rounded corners
	}
	void VobEditor::drawCustomVobHeader(ImGuiID id, ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, bool border, float rounding)
	{
		using namespace ImGui;

		auto& g = *GImGui;

		const ImU32 transparent = ImColor(0.0f, 0.0f, 0.0f, 0.0f);

		ImRect bb;
		bb.Min = p_min;
		bb.Max = p_max;
		bool hovered = g.HoveredId == id;

		if (hovered) {
			// Drag source doesn't report as hovered
			if (hovered && g.DragDropActive && !(g.DragDropSourceFlags & ImGuiDragDropFlags_SourceNoDisableHover))
				fill_col = transparent;
			auto& payload = g.DragDropPayload;
			if (strncmp(payload.DataType, "vob", 3) == 0) {
				bool test = false;
			}
		}

		ImGui::RenderFrame(p_min, p_max, fill_col, border, rounding);
	}
}