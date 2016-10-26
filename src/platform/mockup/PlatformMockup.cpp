#include "platform/mockup/PlatformMockup.hpp"
#include <iostream>

using namespace std;

int PlatformMockup::createWindow(Window::WindowStruct const& desc)
{
	cout << "PlatformMockup::createWindow(): created window.";
	return 0;
}

void PlatformMockup::showWindow(int handle, bool showIt)
{
	cout << "PlatformMockup::showWindow(handle, showIt): window is visible.";
}

void PlatformMockup::destroyWindow(int handle)
{
	cout << "PlatformMockup::destroyWindow(handle): window destroyed.";
}

void PlatformMockup::release()
{
	cout << "PlatformMockup::release(): released allocated memory.";
}