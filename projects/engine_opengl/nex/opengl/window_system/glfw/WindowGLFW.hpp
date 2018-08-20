#pragma once

#include <nex/Window.hpp>
#include <nex/opengl/window_system/glfw/InputGLFW.hpp>


struct GLFWwindow;
class InputGLFW;

class WindowGLFW : public Window
{
public:
	WindowGLFW(WindowStruct const& description);

	WindowGLFW(const WindowGLFW&) = delete;
	WindowGLFW& operator=(const WindowGLFW&) = delete;

	WindowGLFW(WindowGLFW&& o);
	WindowGLFW& operator=(WindowGLFW&&);

	virtual ~WindowGLFW();

	void activate() override;


	void close() override;
	Input* getInputDevice() override;

	GLFWwindow* getSource() const;

	virtual bool hasFocus() override;

	void init();

	bool isOpen() override;
	void minimize() override;

	/**
	* Releases any allocated memory
	*/
	void release();

	void resize(int newWidth, int newHeight) override;
	void setCursorPosition(int xPos, int yPos) override;
	void setFocus(bool focus);
	void setFullscreen() override;

	/**
	 * Sets width and height of the window, but doesn't update the underlying GLFWwindow
	 * (-> no visual update!)
	 */
	void setSize(int width, int height);

	virtual void setTitle(const std::string& newTitle) override;

	void setVisible(bool visible) override;
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