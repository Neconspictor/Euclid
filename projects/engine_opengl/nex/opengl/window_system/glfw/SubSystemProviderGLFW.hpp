#include <nex/logging/LoggingClient.hpp>
#include <nex/SubSystemProvider.hpp>
#include <unordered_map>
#include <unordered_set>
#include <nex/opengl/window_system/glfw/WindowGLFW.hpp>


class SubSystemProviderGLFW : public SubSystemProvider
{
public:

	Window* createWindow(Window::WindowStruct& desc) override;

	std::unique_ptr<ImGUI_Impl> createGUI(Window* window) override;

	static SubSystemProviderGLFW* get();

	bool init() override;

	bool isTerminated() const override;

	void pollEvents() override;

	void terminate() override;

	static void errorCallback(int error, const char* description);


protected:
	bool m_isInitialized;
	nex::LoggingClient logClient;

private:
	SubSystemProviderGLFW();

	std::list<WindowGLFW> windows;
};