#ifndef WINDOW_WIN32_HPP
#define WINDOW_WIN32_HPP
#include "platform/Window.hpp"
#include <Windows.h>
#include <string>
#include <map>

class WindowsInputDevice;

class WindowWin32 : public Window
{
public:
	explicit WindowWin32(WindowStruct const& desc);

	virtual ~WindowWin32();
	void embedRenderer(Renderer* renderer) override;

	void setVisible(bool visible) override;

	void minimize() override;

	void setFullscreen() override;
	
	void setWindowed() override;
	
	void resize(int newWidth, int newHeight) override;

	bool isOpen() override;

	void close() override;

	void pollEvents() override;

	void swapBuffers() override;

	void activate() override;

	Input* getInputDevice() override;

	bool hasFocus() override;

protected:

	virtual void embedOpenGLRenderer(Renderer* renderer);
	static Window* getWindowByHWND(HWND hwnd);

	Input* inputDeviceTest;
	Input* inputDevice;
	Input* sdlInputDevice;

private:
	static std::map<HWND, Window*> windowTable;
	HWND hwnd;			// window handle
	HDC hdc;
	HGLRC openglContext;	// OpenGL Context if opengl is used as renderer
	HWND createWindow(HINSTANCE& hinst, std::string title);
	static LRESULT CALLBACK dispatchInputEvents(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void update();
};

#endif