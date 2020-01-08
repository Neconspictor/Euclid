#include <nex/gui/SceneView.hpp>
#include <nex/gui/ImGUI_Extension.hpp>
#include <nex/gui/Picker.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/scene/Vob.hpp>
#include <nex/gui/vob/VobView.hpp>
#include <nex/gui/vob/VobViewMapper.hpp>
#include <nex/renderer/RenderEngine.hpp>



nex::gui::SceneView::SceneView(Picker* picker, Scene* scene) :
	mPicker(picker),
	mScene(scene)
{
	mSceneRootIcon.texture = TextureManager::get()->getImage("_intern/icon/icon_scene_collection.png", false);
}

nex::gui::SceneView::~SceneView() = default;

void nex::gui::SceneView::setScene(nex::Scene* scene)
{
	mScene = scene;
}

void nex::gui::SceneView::drawSelf()
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


nex::Vob* nex::gui::SceneView::drawVobHierarchy(Vob* vob)
{

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) return nullptr;

	nex::gui::ID vobID((int)vob);

	auto& children = vob->getChildren();

	const char* name = vob->getName().c_str();
	auto* vobView = VobViewMapper::getViewByVob(vob);

	Vob* selectedVob = nullptr;
	ImGui::Bullet();

	if (children.size() > 0) {

		auto open = nex::gui::TreeNodeExCustomShape(name, drawCustomVobHeader,
			true,
			ImVec2(0,0),
			ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_OpenOnArrow); //ImGui::TreeNodeEx(name, ImGuiTreeNodeFlags_OpenOnArrow);
		auto toggled = ImGui::IsItemToggledOpen();
		auto released = ImGui::IsMouseReleased(0);
		auto hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_None);
		drawDragDrop(vob);

		if (vobView->hasIcon()) {
			ImGui::SameLine();
			auto& dc = ImGui::GetCurrentWindow()->DC;
			auto& g = *GImGui;
			dc.CursorPos.x += g.Style.ItemSpacing.x;
			vobView->drawIcon();
		}

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
		static bool edit = false;

		if (edit && mPicker->getPicked() == vob) {

			std::string& name = vob->getName();

			StyleColorPush framebg (ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));

			auto padding = GImGui->Style.FramePadding.x;


			if (ImGui::InputText("##Input field", &name, ImVec2(ImGui::CalcTextSize(name.c_str()).x + padding * 2, 0), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue)) {
				edit = false;
			}


			auto id = ImGui::GetCurrentWindow()->GetID("##Input field");

			ImGui::ActivateItem(id);
		}
		else {
			if (ImGui::ButtonEx(vob->getName().c_str(), ImVec2(0, 0))) { //ImGuiButtonFlags_PressedOnClick
				selectedVob = vob;
			}
			drawDragDrop(vob);

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
				edit = true;
			}
		}

		if (vobView->hasIcon()) {
			ImGui::SameLine();
			vobView->drawIcon();
		}

	}


	return selectedVob;
}



bool nex::gui::SceneView::drawSceneRoot()
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

		auto* window = ImGui::GetCurrentWindow();
		auto width = window->DC.CurrLineSize.y - window->DC.CurrLineTextBaseOffset;

		auto insertPos = window->DC.CursorPos;
		const auto& style = GImGui->Style;
		auto textSizeY = ImGui::CalcTextSize("-").y;
		const float frame_height = ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + style.FramePadding.y * 2), textSizeY + style.FramePadding.y * 2);
		const auto iconSize = frame_height - style.FramePadding.y * 2;
			
		auto offset = ImVec2(iconSize + style.FramePadding.x + 5, 0);


		open = nex::gui::TreeNodeExCustomShape(text, 
			drawCustomRootHeader,
			true,
			offset,
			ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf);
		drawDragDropRoot();	

		auto backupPos = window->DC.CursorPos;

		window->DC.CursorPos = insertPos + ImVec2(style.FramePadding.x, style.FramePadding.y * 0.5);// (frame_height - iconSize) * 0.75);
		ImGui::Image(&mSceneRootIcon, ImVec2(iconSize, iconSize));
		window->DC.CursorPos = backupPos;

	}
		

	return open;
}

void nex::gui::SceneView::endSceneRoot()
{
	ImGui::TreePop();
}

void nex::gui::SceneView::drawDragDrop(Vob* vob)
{
	if (ImGui::BeginDragDropSource()) {

		if (auto* payload = ImGui::GetDragDropPayload()) {
			if (strncmp(payload->DataType, VOB_PAYLOAD, 3) != 0) {
				ImGui::SetDragDropPayload(VOB_PAYLOAD, &vob, sizeof(Vob**));
			}
		}

		ImGui::Text(vob->getName().c_str());

		ImGui::EndDragDropSource();
	}


	nex::gui::DragDropTarget target;

	if (target.isActive()) {
		auto* payload = ImGui::AcceptDragDropPayload(VOB_PAYLOAD);

		if (payload) {
			auto* newChild = *(Vob**)payload->Data;

			// We don't want to add the vob to its own child hierarchy
			if (newChild->hasChild(vob)) return;

			if (auto* oldParent = newChild->getParent()) {
				oldParent->removeChild(newChild);
			}

			nex::RenderEngine::getCommandQueue()->push([=]() {
				mScene->removeActiveRoot(newChild);
				});

			vob->addChild(newChild);
		}
	}
}
void nex::gui::SceneView::drawDragDropRoot()
{
	if (ImGui::BeginDragDropTarget()) {
		auto* payload = ImGui::AcceptDragDropPayload(VOB_PAYLOAD);

		if (payload) {
			auto* newChild = *(Vob**)payload->Data;
			if (auto* oldParent = newChild->getParent()) {
				oldParent->removeChild(newChild);
			}

			nex::RenderEngine::getCommandQueue()->push([=]() {
				mScene->removeActiveVobUnsafe(newChild, false);
				mScene->addActiveVobUnsafe(newChild, false);
				});
		}

		ImGui::EndDragDropTarget();
	}
}

void nex::gui::SceneView::drawDragDropRearrange(int placerIndex)
{

	nex::gui::ID vobID(ImGui::GetID("rearrange"));
	auto region = ImGui::GetContentRegionAvail();

	ImGui::InvisibleButton("", ImVec2(region.x, 4));

	if (ImGui::BeginDragDropTarget()) {
		auto* payload = ImGui::AcceptDragDropPayload(VOB_PAYLOAD);

		if (payload) {
			auto* vob = *(Vob**)payload->Data;
			nex::RenderEngine::getCommandQueue()->push([=]() {
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

				auto newVobIndex = placerIndex; 
					
				if (vobIndex < placerIndex) --newVobIndex;

				roots.erase(roots.begin() + vobIndex);
				roots.insert(roots.begin() + newVobIndex, vob);
				});
		}

		ImGui::EndDragDropTarget();
	}
}

void nex::gui::SceneView::drawCustomRootHeader(ImGuiID id, ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, bool border, float rounding)
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
	draw_list->AddRectFilled(p_min + ImVec2(1, 1), p_max - ImVec2(1, 1), col, 5.0f, corners_tl_br);  // Square with two rounded corners
	draw_list->AddRect(p_min, p_max, blackCol, 5.0f, corners_tl_br, th);  // Square with two rounded corners
}

void nex::gui::SceneView::drawCustomVobHeader(ImGuiID id, ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, bool border, float rounding)
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
		if (strncmp(payload.DataType, VOB_PAYLOAD, 3) == 0) {
			bool test = false;
		}
	}

	ImGui::RenderFrame(p_min, p_max, fill_col, border, rounding);
}

void nex::gui::SceneView::drawIcon(const ImGUI_TextureDesc* desc, bool centerHeight, const ImVec4& tintColor)
{
	auto& g = *GImGui;
	auto* window = ImGui::GetCurrentWindow();
	auto height = window->DC.CurrLineSize.y; //g.FontSize;
		
	if (centerHeight)	window->DC.CursorPos.y += height/2;

	ImGui::Image((void*)desc, ImVec2(height, height), ImVec2(0, 0), ImVec2(1, 1), tintColor);

	if (centerHeight) window->DC.CursorPos.y -= height/2;
}