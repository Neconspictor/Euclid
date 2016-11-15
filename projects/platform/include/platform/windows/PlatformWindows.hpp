#ifndef PLATFORM_WINDOWS_HPP
#define PLATFORM_WINDOWS_HPP
#include <Windows.h>
#include <platform/Platform.hpp>

//if MVSC is used, Visual Leak Detector is used for debugging
#include <Brofiler.h>
#ifdef VDL_USED
#include <vld.h>
#endif
#include <platform/logging/LoggingClient.hpp>

/**
 * The platform class for the windows platform
 */
class PlatformWindows : public Platform
{

public:

	PlatformWindows();

	void setVSync(const Renderer& renderer, bool value) override;

	std::unique_ptr<Window> createWindow(Window::WindowStruct const& desc) override;

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
	void setVSyncOpenGL(bool value);

private:
	platform::LoggingClient logClient;
};

#endif