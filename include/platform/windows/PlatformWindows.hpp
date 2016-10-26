#ifndef PLATFORM_WINDOWS_HPP
#define PLATFORM_WINDOWS_HPP
#include "platform/windows/window/WindowWin32.hpp"
#include "platform/Platform.hpp"

//if MVSC is used, Visual Leak Detector is used for debugging
#include <vld.h>

class PlatformWindows : public Platform
{
public:

	int createWindow(Window::WindowStruct const& desc) override;
	
	void showWindow(int handle, bool showIt) override;
	
	void destroyWindow(int handle) override;
	
	void release() override;

	static void setOpenGLPixelFormat(HDC& hdc);
	static HGLRC createOpenGLContext(HDC& hdc);
	static void destroyOpenGLContext(HGLRC hglrc);
protected:
private:
};

#endif