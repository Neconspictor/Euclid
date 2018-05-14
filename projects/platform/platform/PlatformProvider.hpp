#pragma once
#include <platform/Input.hpp>
#include <platform/Window.hpp>

class PlatformProvider
{
public:

	virtual ~PlatformProvider() {}
	
	virtual Window* createWindow(Window::WindowStruct& desc) = 0;

	virtual Renderer* getRenderer() = 0;

	virtual Input* getInput() = 0;

	virtual bool init() = 0;

	/**
	* Polls and process events for all windows created by this system.
	*/
	virtual void pollEvents() = 0;

	virtual void terminate() = 0;
};
