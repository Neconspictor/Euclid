#ifndef PLATFORM_WINDOWS_HPP
#define PLATFORM_WINDOWS_HPP
#include <Windows.h>
#include <platform/Platform.hpp>

//if MVSC is used, Visual Leak Detector is used for debugging
#include <vld.h>

/**
 * The platform class for the windows platform
 */
class PlatformWindows : public Platform
{
public:

	int createWindow(Window::WindowStruct const& desc) override;
	
	void showWindow(int handle, bool showIt) override;
	
	void destroyWindow(int handle) override;
	
	void release() override;

	/**
	 * Sets the OpenGL pixel format for a given device context.
	 */
	static void setOpenGLPixelFormat(HDC& hdc);
	
	/**
	* creates a new OpenGL context for a given device context.
	*/
	static HGLRC createOpenGLContext(HDC& hdc);

	/**
	* Destroys a given OpenGL context.
	*/
	static void destroyOpenGLContext(HGLRC hglrc);
protected:
private:
};

#endif