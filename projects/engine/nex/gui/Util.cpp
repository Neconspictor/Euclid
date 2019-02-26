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

		const bool log_new_line = ref_pos && (ref_pos->y > window->DC.LogLinePosY + 1);
		if (ref_pos)
			window->DC.LogLinePosY = ref_pos->y;

		const char* text_remaining = text;
		if (g.LogStartDepth > window->DC.TreeDepth)  // Re-adjust padding if we have popped out of our starting depth
			g.LogStartDepth = window->DC.TreeDepth;
		const int tree_depth = (window->DC.TreeDepth - g.LogStartDepth);
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
		float y2 = window->DC.CursorPos.y + window->DC.CurrentLineHeight;
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

		ImGuiSeparatorFlags flags = 0;

		if (vertical)
			flags |= ImGuiSeparatorFlags_Vertical;

		if ((flags & (ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_Vertical)) == 0)
			flags |= (window->DC.LayoutType == ImGuiLayoutType_Horizontal) ? ImGuiSeparatorFlags_Vertical : ImGuiSeparatorFlags_Horizontal;
		IM_ASSERT(ImIsPowerOfTwo((int)(flags & (ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_Vertical))));   // Check that only 1 option is selected
		if (flags & ImGuiSeparatorFlags_Vertical)
		{
			VerticalSeparatorThickness(thickness);
			return;
		}

		// Horizontal Separator
		if (window->DC.ColumnsSet)
			PopClipRect();

		float x1 = window->Pos.x;
		float x2 = window->Pos.x + window->Size.x;
		if (!window->DC.GroupStack.empty())
			x1 += window->DC.IndentX;

		const ImRect bb(ImVec2(x1, window->DC.CursorPos.y), ImVec2(x2, window->DC.CursorPos.y + thickness));
		ItemSize(ImVec2(0.0f, 0.0f)); // NB: we don't provide our width so that it doesn't get feed back into AutoFit, we don't provide height to not alter layout.
		if (!ItemAdd(bb, 0))
		{
			if (window->DC.ColumnsSet)
				PushColumnClipRect();
			return;
		}

		window->DrawList->AddLine(bb.Min, ImVec2(bb.Max.x, bb.Min.y), GetColorU32(ImGuiCol_Separator), thickness);

		if (g.LogEnabled)
			LogRenderedText(NULL, IM_NEWLINE "--------------------------------");

		if (window->DC.ColumnsSet)
		{
			PushColumnClipRect();
			window->DC.ColumnsSet->LineMinY = window->DC.CursorPos.y;
		}
	}

	void Vector3D(glm::vec3* vec, const char* label)
	{
		std::stringstream ss;
		ss << label << ":";
		ImGui::LabelText("", ss.str().c_str());

		ImGui::PushID(label);
		ImGui::DragFloat("###X", &vec->x);
		ImGui::DragFloat("###Y", &vec->y);
		ImGui::DragFloat("###Z", &vec->z);
		ImGui::PopID();
	}
}
