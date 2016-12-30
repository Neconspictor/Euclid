#pragma once
#include <platform/Input.hpp>
#include <platform/Window.hpp>

class WindowSystem
{
public:

	virtual ~WindowSystem() {}
	
	virtual Window* createWindow(Window::WindowStruct& desc, Renderer& renderer) = 0;

	virtual bool init() = 0;

	/**
	* Polls and process events for all windows created by this system.
	*/
	virtual void pollEvents() = 0;

	virtual void terminate() = 0;
};
