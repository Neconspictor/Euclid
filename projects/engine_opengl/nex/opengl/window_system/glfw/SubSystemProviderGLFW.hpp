#include <nex/logging/LoggingClient.hpp>
#include <nex/SubSystemProvider.hpp>
#include <unordered_map>
#include <unordered_set>
#include <nex/opengl/window_system/glfw/WindowGLFW.hpp>


class SubSystemProviderGLFW : public SubSystemProvider
{
public:

	Window* createWindow(Window::WindowStruct& desc) override;

	virtual std::unique_ptr<ImGUI_Impl> createGUI(Window* window) override;

	RenderBackend* getRenderBackend() override;

	Input* getInput() override;

	static SubSystemProviderGLFW* get();

	virtual bool init() override;

	virtual void pollEvents() override;

	virtual void terminate() override;

	static void errorCallback(int error, const char* description);


protected:
	bool m_isInitialized;
	nex::LoggingClient logClient;

private:
	SubSystemProviderGLFW();

	std::list<WindowGLFW> windows;
};