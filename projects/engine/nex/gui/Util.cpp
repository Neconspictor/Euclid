#include <nex/gui/Util.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace nex::gui
{
	using namespace ImGui;

	// Internal version that takes a position to decide on newline placement and pad items according to their depth.
	// We split text into individual lines to add current tree level padding
	static void LogRenderedText(const ImVec2* ref_pos, const char* text, const char* text_end = NULL)
	{
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = g.CurrentWindow;

		if (!text_end)
			text_end = ImGui::FindRenderedTextEnd(text, text_end);

		const bool log_new_line = ref_pos && (ref_pos->y > g.LogLinePosY + 1);
		if (ref_pos)
			g.LogLinePosY = ref_pos->y;

		const char* text_remaining = text;
		if (g.LogDepthRef > window->DC.TreeDepth)  // Re-adjust padding if we have popped out of our starting depth
			g.LogDepthRef = window->DC.TreeDepth;
		const int tree_depth = (window->DC.TreeDepth - g.LogDepthRef);
		for (;;)
		{
			// Split the string. Each new line (after a '\n') is followed by spacing corresponding to the current depth of our log entry.
			const char* line_end = text_remaining;
			while (line_end < text_end)
				if (*line_end == '\n')
					break;
				else
					line_end++;
			if (line_end >= text_end)
				line_end = NULL;

			const bool is_first_line = (text == text_remaining);
			bool is_last_line = false;
			if (line_end == NULL)
			{
				is_last_line = true;
				line_end = text_end;
			}
			if (line_end != NULL && !(is_last_line && (line_end - text_remaining) == 0))
			{
				const int char_count = (int)(line_end - text_remaining);
				if (log_new_line || !is_first_line)
					ImGui::LogText(IM_NEWLINE "%*s%.*s", tree_depth * 4, "", char_count, text_remaining);
				else
					ImGui::LogText(" %.*s", char_count, text_remaining);
			}

			if (is_last_line)
				break;
			text_remaining = line_end + 1;
		}
	}


	void VerticalSeparatorThickness(float thickness)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;
		ImGuiContext& g = *GImGui;

		float y1 = window->DC.CursorPos.y;
		float y2 = window->DC.CursorPos.y + window->DC.CurrLineSize.y;
		const ImRect bb(ImVec2(window->DC.CursorPos.x, y1), ImVec2(window->DC.CursorPos.x + thickness, y2));
		ItemSize(ImVec2(bb.GetWidth(), 0.0f));
		if (!ItemAdd(bb, 0))
			return;

		window->DrawList->AddLine(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Min.x, bb.Max.y), GetColorU32(ImGuiCol_Separator), thickness);
		if (g.LogEnabled)
			LogText(" |");
	}

	void Separator(float thickness, bool vertical)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImGuiContext& g = *GImGui;

		ImGuiSeparatorFlags_ flags = ImGuiSeparatorFlags_Horizontal;
		if (vertical)flags = ImGuiSeparatorFlags_Vertical;

		float thickness_draw = thickness;
		float thickness_layout = 0.0f;
		if (flags & ImGuiSeparatorFlags_Vertical)
		{
			// Vertical separator, for menu bars (use current line height). Not exposed because it is misleading and it doesn't have an effect on regular layout.
			float y1 = window->DC.CursorPos.y;
			float y2 = window->DC.CursorPos.y + window->DC.CurrLineSize.y;
			const ImRect bb(ImVec2(window->DC.CursorPos.x, y1), ImVec2(window->DC.CursorPos.x + thickness_draw, y2));
			ItemSize(ImVec2(thickness_layout, 0.0f));
			if (!ItemAdd(bb, 0))
				return;

			// Draw
			window->DrawList->AddLine(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Min.x, bb.Max.y), GetColorU32(ImGuiCol_Separator));
			if (g.LogEnabled)
				LogText(" |");
		}
		else if (flags & ImGuiSeparatorFlags_Horizontal)
		{
			// Horizontal Separator
			float x1 = window->Pos.x;
			float x2 = window->Pos.x + window->Size.x;
			if (!window->DC.GroupStack.empty())
				x1 += window->DC.Indent.x;

			ImGuiColumns* columns = (flags & ImGuiSeparatorFlags_SpanAllColumns) ? window->DC.CurrentColumns : NULL;
			if (columns)
				PushColumnsBackground();

			// We don't provide our width to the layout so that it doesn't get feed back into AutoFit
			const ImRect bb(ImVec2(x1, window->DC.CursorPos.y), ImVec2(x2, window->DC.CursorPos.y + thickness_draw));
			ItemSize(ImVec2(0.0f, thickness_layout));
			if (!ItemAdd(bb, 0))
			{
				if (columns)
				{
					PopColumnsBackground();
					columns->LineMinY = window->DC.CursorPos.y;
				}
				return;
			}

			// Draw
			window->DrawList->AddLine(bb.Min, ImVec2(bb.Max.x, bb.Min.y), GetColorU32(ImGuiCol_Separator));
			if (g.LogEnabled)
				LogRenderedText(&bb.Min, "--------------------------------");

			if (columns)
			{
				PopColumnsBackground();
				columns->LineMinY = window->DC.CursorPos.y;
			}
		}
	}

	bool Vector3D(glm::vec3* vec, const char* label, float speed)
	{
		//std::stringstream ss;
		//ss << label << ":";
		//ImGui::TextUnformatted(ss.str().c_str());
		return ImGui::DragFloat3(label, (float*)vec);
	}

	void EulerRot(glm::vec3* vec, const char* label, float speed)
	{
		std::stringstream ss;
		ss << label << ":";
		ImGui::TextUnformatted(ss.str().c_str());

		ImGui::PushID(label);
		ImGui::TextUnformatted("X: ");
		ImGui::SameLine();
		ImGui::DragFloat("###X", &vec->x, speed);

		ImGui::TextUnformatted("Y: ");
		ImGui::SameLine();
		ImGui::DragFloat("###Y", &vec->y, speed);
		
		
		ImGui::TextUnformatted("Z: ");
		ImGui::SameLine();
		ImGui::DragFloat("###Z", &vec->z, speed);
		ImGui::PopID();
	}

	void Quat(glm::quat* quat, const char* label)
	{
		std::stringstream ss;
		ss << label << ":";
		ImGui::TextUnformatted(ss.str().c_str());

		ImGui::PushID(label);
		ImGui::TextUnformatted("X: ");
		ImGui::SameLine();
		ImGui::DragFloat("###X", &quat->x, 0.1f);
		ImGui::TextUnformatted("Y: ");
		ImGui::SameLine();
		ImGui::DragFloat("###Y", &quat->y, 0.1f);
		ImGui::TextUnformatted("Z: ");
		ImGui::SameLine();
		ImGui::DragFloat("###Z", &quat->z, 0.1f);
		ImGui::TextUnformatted("W: ");
		ImGui::SameLine();
		ImGui::DragFloat("###W", &quat->w, 0.1f);
		ImGui::PopID();
	}
}
