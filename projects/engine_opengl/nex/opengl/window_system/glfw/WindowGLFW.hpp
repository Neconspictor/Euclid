#pragma once

#include <nex/Window.hpp>
#include <nex/opengl/window_system/glfw/InputGLFW.hpp>

struct GLFWwindow;

namespace nex
{
	class InputGLFW;

	class WindowGLFW : public Window
	{
	public:
		WindowGLFW(WindowStruct const& description);

		WindowGLFW(const WindowGLFW&) = delete;
		WindowGLFW& operator=(const WindowGLFW&) = delete;

		WindowGLFW(WindowGLFW&& o);
		WindowGLFW& operator=(WindowGLFW&&);

		void activate() override;


		void close() override;

		void* getNativeWindow() override;

		Input* getInputDevice() override;

		GLFWwindow* getSource() const;

		bool hasFocus() override;

		void init();

		bool isOpen() override;
		void minimize() override;

		/**
		* Releases any allocated memory
		*/
		void release();

		void reopen() override;

		void resize(int newWidth, int newHeight) override;
		void setCursorPosition(int xPos, int yPos) override;
		void setFocus(bool focus);
		void setFullscreen() override;

		/**
		 * Sets width and height of the window, but doesn't update the underlying GLFWwindow
		 * (-> no visual update!)
		 */
		void setSize(int width, int height);

		void setTitle(const std::string& newTitle) override;

		void setVisible(bool visible) override;
		void setVsync(bool vsync) override;
		void setWindowed() override;
		void showCursor(bool show) override;
		void swapBuffers() override;


	protected:
		void refreshWindowWithoutCallbacks();

		friend class SubSystemProviderGLFW;
		friend class InputGLFW;

		GLFWwindow* window;
		InputGLFW inputDevice;
		bool m_hasFocus;

		void createOpenGLWindow();
	};
}