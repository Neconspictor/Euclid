#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#define PLATFORM_WINDOWS

#include <platform/Window.hpp>


/**
 * A platform represents the environment the application is running on. A platform object wraps the 
 * OS specific code and provides a unified interface to os specific things like windows and device input
 * events.
 */
class Platform
{
public:
	virtual ~Platform(){}


	/**
	 * Creates a new window and returns a handle for this window.
	 */
	virtual int createWindow(Window::WindowStruct const& desc) = 0;

	/**
	 * Sets the visibility of a window specified by its handle. 
	 */
	virtual void showWindow(int handle, bool showIt) = 0;

	/**
	 * Destroys a given window by its handle
	 */
	virtual void destroyWindow(int handle) = 0;

	/**
	 * Releases all memory allocated by this platform object.
	 */
	virtual void release() = 0;
};

#endif