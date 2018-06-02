#pragma once
#include <platform/gui/ImGUI.hpp>
#include <string>

class WindowGLFW;
struct GLFWcursor;

class ImGUI_GL : public ImGUI_Impl
{
public:
	ImGUI_GL(WindowGLFW& window, std::string glsl_version = "#version 150");

	virtual ~ImGUI_GL();

	virtual void newFrame() override;

	virtual void renderDrawData(ImDrawData* draw_data) override;

	virtual void shutdown() override;



	static const char* getClipboardText(void* user_data);

	static void setClipboardText(void* user_data, const char* text);

protected:
	void init();

protected:
	WindowGLFW* window;
	std::string glsl_version;
	GLFWcursor*  g_MouseCursors[ImGuiMouseCursor_COUNT];
	bool         g_MouseJustPressed[3];
};