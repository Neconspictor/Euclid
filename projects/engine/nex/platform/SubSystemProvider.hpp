#pragma once
#include <nex/platform/Window.hpp>

namespace nex
{
	class SubSystemProvider
	{
	public:

		virtual ~SubSystemProvider() = default;

		virtual Window* createWindow(Window::WindowStruct& desc) = 0;

		virtual bool init() = 0;

		virtual bool isTerminated() const = 0;

		/**
		* Polls and process events for all windows created by this system.
		*/
		virtual void pollEvents() = 0;

		virtual void terminate() = 0;

		virtual void waitForEvents() = 0;
	};
}