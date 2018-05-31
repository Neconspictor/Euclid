#include <platform/logging/LoggingClient.hpp>
#include <platform/PlatformProvider.hpp>
#include <unordered_map>
#include <unordered_set>
#include <window_system/glfw/WindowGLFW.hpp>


class SubSystemProviderGLFW : public SubSystemProvider
{
public:

	Window* createWindow(Window::WindowStruct& desc) override;

	virtual ImGUI_Impl* createGUI(Window* window) override;

	Renderer* getRenderer() override;

	Input* getInput() override;

	static SubSystemProviderGLFW* get();

	virtual bool init() override;

	virtual void pollEvents() override;

	virtual void terminate() override;

	static void errorCallback(int error, const char* description);


protected:
	bool m_isInitialized;
	platform::LoggingClient logClient;

private:
	SubSystemProviderGLFW();

	static SubSystemProviderGLFW instance;

	std::list<WindowGLFW> windows;
};