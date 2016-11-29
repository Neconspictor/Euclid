#ifndef WINDOW_WIN32_HPP
#define WINDOW_WIN32_HPP
#include <platform/Window.hpp>
#include <Windows.h>
#include <string>
#include <map>
#include <platform/windows/input/SDL/SDLInputDevice.hpp>

class WindowsInputDevice;

/**
 * A window class for the Windows platform.
 */
class WindowWin32 : public Window
{
public:
	explicit WindowWin32(WindowStruct const& desc);

	virtual ~WindowWin32();

	void activate() override;

	void close() override;

	void embedRenderer(std::shared_ptr<Renderer>& renderer) override;

	Input* getInputDevice() override;

	bool hasFocus() override;

	bool isOpen() override;

	void minimize() override;

	void pollEvents() override;

	void resize(int newWidth, int newHeight) override;

	void setCursorPosition(int xPos, int yPos) override;

	void setFullscreen() override;

	void setTitle(const std::string& newTitle) override;

	void setVisible(bool visible) override;

	void setWindowed() override;

	void swapBuffers() override;

protected:

	virtual void embedOpenGLRenderer(std::shared_ptr<Renderer>& renderer);
	static Window* getWindowByHWND(HWND hwnd);

	SDLInputDevice* sdlInputDevice;

private:
	static std::map<HWND, Window*> windowTable;
	HWND hwnd;			// window handle
	HDC hdc;
	HGLRC openglContext;	// OpenGL Context if opengl is used as renderer
	HWND createWindow(HINSTANCE& hinst, std::string title);
	static LRESULT CALLBACK dispatchInputEvents(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void update() const;
};

#endif