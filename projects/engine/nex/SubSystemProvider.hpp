#pragma once
#include <nex/Input.hpp>
#include <nex/Window.hpp>
#include <nex/gui/ImGUI.hpp>

class SubSystemProvider
{
public:

	virtual ~SubSystemProvider() {}
	
	virtual Window* createWindow(Window::WindowStruct& desc) = 0;

	virtual std::unique_ptr<ImGUI_Impl> createGUI(Window* window) = 0;

	virtual RenderBackend* getRenderBackend() = 0;

	virtual Input* getInput() = 0;

	virtual bool init() = 0;

	virtual bool isTerminated() const = 0;

	/**
	* Polls and process events for all windows created by this system.
	*/
	virtual void pollEvents() = 0;

	virtual void terminate() = 0;
};
