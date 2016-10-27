#ifndef PLATFORM_MOCKUP_HPP
#define PLATFORM_MOCKUP_HPP
#include "platform/mockup/window/WindowMockup.hpp"
#include "platform/Platform.hpp"

class PlatformMockup : public Platform
{
public:
	int createWindow(Window::WindowStruct const& desc) override;
	void showWindow(int handle, bool showIt) override;
	void destroyWindow(int handle) override;
	void release() override;
};

#endif