#pragma once
#include "nex/math/Constant.hpp"
#include <nex/gui/ImGUI.hpp>

namespace nex::gui
{
	/**
	 * ImGui functions wrapped into classes for leveraging RAII
	 */

	struct ID {
		ID(int id);
		ID(const ID&) = delete;
		ID(ID&&) = delete;
		ID& operator=(const ID&) = delete;
		ID& operator=(ID&&) = delete;
		~ID();
	};

	struct StyleColorPush {
		StyleColorPush(ImGuiCol idx, const ImVec4& col);
		StyleColorPush(const StyleColorPush&) = delete;
		StyleColorPush(StyleColorPush&&) = delete;
		StyleColorPush& operator=(const StyleColorPush&) = delete;
		StyleColorPush& operator=(StyleColorPush&&) = delete;
		~StyleColorPush();
	};

	struct DragDropTarget {
		DragDropTarget();
		DragDropTarget(const DragDropTarget&) = delete;
		DragDropTarget(DragDropTarget&&) = delete;
		DragDropTarget& operator=(const DragDropTarget&) = delete;
		DragDropTarget& operator=(DragDropTarget&&) = delete;
		~DragDropTarget();

		bool isActive() const;

	private:
		bool mIsActive;
	};

	/**
	 * New and customized ImGui functions 
	 */

	void Separator(float thickness, bool vertical = false);

	bool Vector3D(glm::vec3* vec, const char* label, float speed = 1.0f);

	void EulerRot(glm::vec3* vec, const char* label, float speed = 1.0f);

	void Quat(glm::quat* quat, const char* label);

	ImVec2 GetWindowContentPadding();
	ImVec2 GetWindowContentEffectiveSize();


	using CustomShapeRenderFunc = std::function<void(ImGuiID id, ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, bool border, float rounding)>;

	bool TreeNodeExCustomShape(const char* label, 
		const CustomShapeRenderFunc& renderFunc, 
		bool clipFrameToContent, 
		ImVec2 offset = ImVec2(0,0),
		ImGuiTreeNodeFlags flags = 0);
	bool TreeNodeBehaviourCustomShape(ImGuiID id, 
		ImGuiTreeNodeFlags flags, 
		const char* label, 
		const char* label_end, 
		const CustomShapeRenderFunc& renderFunc, 
		bool clipFrameToContent,
		ImVec2 offset);

	bool BeginImageMenuCustom(const char* labelID, const ImGUI_TextureDesc& textureDesc, ImVec2 size = ImVec2(0,0), bool tightSpanning = true, bool enabled = true);



	bool Splitter(const char* label, bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f);
};
