#pragma once
//#include <nanogui/screen.h>


class SubSystemProviderGLFW;
class WindowGLFW;
class Window;

class NanoGUI
{
public:
	NanoGUI(SubSystemProviderGLFW* windowSystem);

	virtual ~NanoGUI();

	virtual void init(Window* window);

	virtual void frameUpdate();

protected:
	SubSystemProviderGLFW* windowSystem;
	WindowGLFW* window;
	//static nanogui::Screen* screen;
};
