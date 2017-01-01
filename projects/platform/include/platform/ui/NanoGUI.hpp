#pragma once
#include <platform/SystemUI.hpp>
#include <nanogui/screen.h>


class WindowSystemGLFW;
class WindowGLFW;

class NanoGUI : public SystemUI
{
public:
	NanoGUI(WindowSystemGLFW* windowSystem);

	virtual ~NanoGUI();

	virtual void init(Window* window) override;

	virtual void frameUpdate() override;

protected:
	WindowSystemGLFW* windowSystem;
	WindowGLFW* window;
	static nanogui::Screen* screen;
};
