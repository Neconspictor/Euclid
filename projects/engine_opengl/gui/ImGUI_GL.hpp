#pragma once
#include <platform/gui/ImGUI.hpp>
#include <string>
#include <platform/logging/LoggingClient.hpp>
#include <glad/glad.h>

class WindowGLFW;
struct GLFWcursor;

class ImGUI_GL : public ImGUI_Impl
{
public:
	ImGUI_GL(WindowGLFW& window, std::string glsl_version = "#version 150");

	virtual ~ImGUI_GL();

	virtual void newFrame() override;

	virtual void renderDrawData(ImDrawData* draw_data) override;

	void shutdown();



	static const char* getClipboardText(void* user_data);

	static void setClipboardText(void* user_data, const char* text);

protected:
	void init();

	bool createDeviceObjects();

	void createFontsTexture();

protected:
	WindowGLFW* window;
	std::string glsl_version;
	GLFWcursor*  g_MouseCursors[ImGuiMouseCursor_COUNT];
	bool         g_MouseJustPressed[3];
	double g_Time;
	GLuint g_FontTexture;
	GLuint g_ShaderHandle; 
	GLuint g_VertHandle;
	GLuint g_FragHandle;
	GLint  g_AttribLocationTex; 
	GLint  g_AttribLocationProjMtx;
	GLint  g_AttribLocationPosition;
	GLint g_AttribLocationUV; 
	GLint g_AttribLocationColor;
	GLuint g_VboHandle;
	GLuint g_ElementsHandle;
	platform::LoggingClient logClient;
};