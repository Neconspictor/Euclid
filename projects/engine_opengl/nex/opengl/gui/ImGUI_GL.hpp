#pragma once
#include <nex/gui/ImGUI.hpp>
#include <nex/common/Log.hpp>

namespace nex
{
	class Window;
	class Cursor;
	class Texture2D;
	class VertexArray;
	class IndexBuffer;
	class VertexBuffer;
}

namespace nex::gui
{
	class ImGUI_GL : public gui::ImGUI_Impl
	{
	public:
		ImGUI_GL(nex::Window* window);

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

		void bindTextureShader(ImGUI_ImageDesc* texture, const glm::mat4& projection);

		bool createDeviceObjects();

		void createFontsTexture();

	protected:

		class Drawer;

		nex::Window* mWindow;
		std::unique_ptr<Cursor> mMouseCursors[ImGuiMouseCursor_COUNT];
		bool         g_MouseJustPressed[3];
		std::unique_ptr<Texture2D> mFontTexture;
		ImGUI_ImageDesc mFontDesc;
		//GLuint g_VboHandle;
		std::unique_ptr<VertexArray> mVertexArray;
		std::unique_ptr<VertexBuffer> mVertexBuffer;
		std::unique_ptr<IndexBuffer> mIndices;
		std::unique_ptr<Drawer> mShaderTexture2D;
		std::unique_ptr<Drawer> mShaderCubeMap;
		nex::Logger m_logger;
	};
}