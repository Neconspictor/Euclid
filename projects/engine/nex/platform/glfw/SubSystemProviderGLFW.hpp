#pragma once

#include <nex/platform/SubSystemProvider.hpp>
#include <nex/platform/glfw/WindowGLFW.hpp>

namespace nex
{
	class SubSystemProviderGLFW : public SubSystemProvider
	{
	public:

		Window* createWindow(Window::WindowStruct& desc) override;

		static SubSystemProviderGLFW* get();

		bool init() override;

		bool isTerminated() const override;

		void pollEvents() override;

		void terminate() override;

		void waitForEvents() override;

		static void errorCallback(int error, const char* description);


	protected:
		bool m_isInitialized;
		nex::Logger m_logger;

	private:
		SubSystemProviderGLFW();

		std::list<WindowGLFW> windows;
	};
}