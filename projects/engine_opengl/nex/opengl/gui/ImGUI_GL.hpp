#pragma once
#include <nex/gui/ImGUI.hpp>
#include <string>
#include <glad/glad.h>
#include <nex/common/Log.hpp>

namespace nex
{
	class Window;
	class Cursor;
}

namespace nex::gui
{

	class ImGUI_GL : public gui::ImGUI_Impl
	{
	public:
		ImGUI_GL(nex::Window* window, std::string glsl_version = "#version 150");

		ImGUI_GL(const ImGUI_GL&) = delete;
		ImGUI_GL& operator=(const ImGUI_GL&) = delete;

		ImGUI_GL(ImGUI_GL&& o) = delete;
		ImGUI_GL& operator=(ImGUI_GL&& o) = delete;

		virtual ~ImGUI_GL();

		void newFrame(float frameTime) override;

		void renderDrawData(ImDrawData* draw_data) override;

		void shutdown();

	protected:

		static const char* getClipboardText(void* inputDevice);
		static void setClipboardText(void* inputDevice, const char* text);

		void init();

		bool createDeviceObjects();

		void createFontsTexture();

	protected:
		nex::Window* mWindow;
		std::string glsl_version;
		std::unique_ptr<Cursor> mMouseCursors[ImGuiMouseCursor_COUNT];
		bool         g_MouseJustPressed[3];
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
		nex::Logger m_logger;
	};
}