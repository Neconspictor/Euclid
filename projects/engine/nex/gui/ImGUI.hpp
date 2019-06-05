#pragma once

#include <imgui/imgui.h>

namespace nex::gui
{
	/**
	 * An interface for concrete render implementations of ImGUI
	 * https://github.com/ocornut/imgui
	 */
	class ImGUI_Impl
	{
	public:
		virtual ~ImGUI_Impl() = default;

		ImGUI_Impl() = default;
		ImGUI_Impl(const ImGUI_Impl&) = delete;
		ImGUI_Impl& operator=(const ImGUI_Impl&) = delete;

		ImGUI_Impl(ImGUI_Impl&&) = default;
		ImGUI_Impl& operator=(ImGUI_Impl&&) = default;

		virtual void newFrame(float frameTime) = 0;

		static bool isActive();

		virtual void renderDrawData(ImDrawData* draw_data) = 0;
	};
}
