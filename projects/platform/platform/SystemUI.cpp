#include <platform/SystemUI.hpp>
#include <platform/window_system/glfw/WindowSystemGLFW.hpp>
#include <platform/ui/NanoGUI.hpp>

SystemUI* SystemUI::instance = nullptr;

SystemUI* SystemUI::get(PlatformProvider* windowSystem)
{
	if (dynamic_cast<WindowSystemGLFW*>(windowSystem) && !instance)
	{
		instance = new NanoGUI(static_cast<WindowSystemGLFW*>(windowSystem));
	}

	return instance;
}

void SystemUI::shutdown()
{
	if (instance) delete instance;
	instance = nullptr;
}

SystemUI::SystemUI()
{
}

SystemUI::~SystemUI()
{
}
