#include <platform/logging/LoggingClient.hpp>
#include <platform/PlatformProvider.hpp>
#include <unordered_map>
#include <unordered_set>
#include <platform/window_system/glfw/WindowGLFW.hpp>


class WindowSystemGLFW : public PlatformProvider
{
public:

	Window* createWindow(Window::WindowStruct& desc) override;

	Renderer* getRenderer() override;

	Input* getInput() override;

	static WindowSystemGLFW* get();

	virtual bool init() override;

	virtual void pollEvents() override;

	virtual void terminate() override;

	static void errorCallback(int error, const char* description);


protected:
	bool m_isInitialized;
	platform::LoggingClient logClient;

private:
	WindowSystemGLFW();

	static WindowSystemGLFW instance;

	std::list<WindowGLFW> windows;
};