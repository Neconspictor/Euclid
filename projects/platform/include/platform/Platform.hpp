#pragma once

#include <platform/Window.hpp>


/**
 * A platform represents the environment the application is running on. A platform object wraps the 
 * OS specific code and provides a unified interface to os specific things like windows and device input
 * events.
 * 
 * Additionally this class provides a method to get a platform object for the current active platform!
 */
class Platform
{
public:
	virtual ~Platform(){}

	virtual void setVSync(const Renderer& renderer, bool value) = 0;

	/**
	* Provides access to the current active platform.
	*/
	static std::shared_ptr<Platform> getActivePlatform();

private:
	static std::shared_ptr<Platform> singleton;

};