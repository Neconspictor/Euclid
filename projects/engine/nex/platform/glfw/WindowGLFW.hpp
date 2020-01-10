#pragma once

#include <nex/platform/Window.hpp>
#include <nex/platform/glfw/InputGLFW.hpp>
#include <GLFW/glfw3.h>

struct GLFWwindow;

namespace nex
{
	class InputGLFW;

	enum class StandardCursorTypeGLFW
	{
		Arrow = GLFW_ARROW_CURSOR,
		Hand = GLFW_HAND_CURSOR,
		TextIBeam = GLFW_IBEAM_CURSOR,
		CrossHair = GLFW_CROSSHAIR_CURSOR,
		HorizontalResize = GLFW_HRESIZE_CURSOR,
		VerticalResize = GLFW_VRESIZE_CURSOR
	};

	enum class CursorStateGLFW
	{
		Disabled = GLFW_CURSOR_DISABLED,
		Hidden = GLFW_CURSOR_HIDDEN,
		Normal = GLFW_CURSOR_NORMAL,
	};

	StandardCursorTypeGLFW translate(StandardCursorType type);
	CursorStateGLFW translate(CursorState state);
	CursorState translate(CursorStateGLFW state);

	class Cursor::Impl 
	{
	public:
		Impl(StandardCursorTypeGLFW shape);
		~Impl();
		Impl(const Impl&) = delete;
		Impl(Impl&&) = delete;
		Impl& operator=(const Impl&) = delete;
		Impl& operator=(Impl&&) = delete;

		GLFWcursor* getCursor();

	private:
		GLFWcursor* mCursor;
	};

	class WindowGLFW : public Window
	{
	public:
		WindowGLFW(WindowStruct const& description);

		WindowGLFW(const WindowGLFW&) = delete;
		WindowGLFW& operator=(const WindowGLFW&) = delete;

		WindowGLFW(WindowGLFW&& o);
		WindowGLFW& operator=(WindowGLFW&&);

		void activate(bool deactivate = false) override;


		void close(bool silent = false) override;

		const Cursor* getCursor() const override;

		CursorState getCursorState() const override;

		void* getNativeWindow() override;

		Input* getInputDevice() override;

		GLFWwindow* getSource() const;

		bool hasFocus() const override;

		void init();

		bool isOpen() const override;
		void minimize() override;

		/**
		* Releases any allocated memory
		*/
		void release();

		void reopen() override;

		void resize(unsigned newWidth, unsigned newHeight) override;

		void setCursor(Cursor* cursor) override;

		void setCursorPosition(int xPos, int yPos) override;
		void setFocus(bool focus);
		void setFullscreen() override;

		/**
		 * Sets framebuffer width and height of the window, but doesn't update the underlying GLFWwindow
		 * (-> no visual update!)
		 */
		void setFrameBufferSize(unsigned width, unsigned height);

		/**
		 * Sets the virtual screen dimensions of the window, but doesn't update the underlying GLFWwindow
		 * (-> no visual update!)
		 */
		void setVirtualScreenDimension(unsigned width, unsigned height);

		void setTitle(const std::string& newTitle) override;

		void setVisible(bool visible) override;
		void setVsync(bool vsync) override;
		void setWindowed() override;
		void showCursor(CursorState state) override;
		void swapBuffers() override;


	protected:
		void refreshWindowWithoutCallbacks();

		friend class SubSystemProviderGLFW;
		friend class InputGLFW;

		GLFWwindow* window;
		InputGLFW inputDevice;
		bool m_hasFocus;
		Cursor* mCursor;

		/**
		 * NOTE: This function has to be implemented by the render backend!
		 */
		void createWindowWithRenderContext();
	};
}