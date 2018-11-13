#pragma once

class ImGUI_Impl;
class Input;
class RenderBackend;

#include <nex/Window.hpp>

class SubSystemProvider
{
public:

	virtual ~SubSystemProvider() {}
	
	virtual Window* createWindow(Window::WindowStruct& desc) = 0;

	virtual std::unique_ptr<ImGUI_Impl> createGUI(Window* window) = 0;

	virtual bool init() = 0;

	virtual bool isTerminated() const = 0;

	/**
	* Polls and process events for all windows created by this system.
	*/
	virtual void pollEvents() = 0;

	virtual void terminate() = 0;
};
