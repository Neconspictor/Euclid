#pragma once
#include <platform/Window.hpp>
#include <platform/window_system/sdl/InputDeviceSDL.hpp>

class WindowSDL : public Window
{
public:
	void activate() override;
	void close() override;
	Input* getInputDevice() override;
	InputDeviceSDL* getInputDeviceSDL();

	bool hasFocus() override;
	bool isOpen() override;
	void minimize() override;
	void resize(int newWidth, int newHeight) override;
	void setCursorPosition(int xPos, int yPos) override;
	void setFullscreen() override;
	void setVisible(bool visible) override;
	void setWindowed() override;
	void swapBuffers() override;

protected:

	friend class WindowSystemSDL;

	WindowSDL(WindowStruct const& description, Renderer& renderer);

	void createOpenGLWindow(WindowStruct const& desc);
	void createNoAPIWindow(WindowStruct const& desc);

	bool closed;

	InputDeviceSDL inputDevice;
	platform::LoggingClient logClient;

	SDL_Window* window;
};