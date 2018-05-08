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

	static Input::Button toButton(int glfwButton);
	static int toGLFWbutton(Input::Button button);
	static int toGLFWkey(Input::Key);
	static Input::Key toKey(int glfwKey);

	virtual void terminate() override;



	static void errorCallback(int error, const char* description);


protected:
	void initInputButtonMap();
	static void initInputKeyMap();


	bool m_isInitialized;
	platform::LoggingClient logClient;

private:
	WindowSystemGLFW();

	static WindowSystemGLFW instance;

	std::list<WindowGLFW> windows;

	// mapping glfw button <-> input button
	static std::unordered_map<int, Input::Button> glfwToButtonMap;
	static std::unordered_map<Input::Button, int> buttonToGlfwMap;

	// mapping glfw key <-> input key
	static std::unordered_map<int, Input::Key> glfwToKeyMap;
	static std::unordered_map<Input::Key, int> keyToGlfwMap;
};