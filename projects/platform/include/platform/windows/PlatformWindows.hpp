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

	/**
	* creates a new OpenGL context for a given device context.
	*/
	HGLRC createOpenGLContext(HDC& hdc);

	std::unique_ptr<Window> createWindow(Window::WindowStruct const& desc) override;

	/**
	* Destroys a given OpenGL context.
	*/
	void destroyOpenGLContext(HGLRC hglrc);

	/**
	 * Provides the active platform object as a PlatformWindows object.
	 * NOTE: If the active platform object isn't of type PlatformWindows,
	 * an UnexpectedPLatformException is thrown.
	 */
	static PlatformWindows* getActivePlatform();
	
	std::string GetLastErrorAsString();
	/**
	* Sets the OpenGL pixel format for a given device context.
	*/
	void setOpenGLPixelFormat(HDC& hdc);

	void setVSync(const Renderer& renderer, bool value) override;

protected:
	void setVSyncOpenGL(bool value);

private:
	platform::LoggingClient logClient;
};

#endif