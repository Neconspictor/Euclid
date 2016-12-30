#include <platform/logging/LoggingClient.hpp>
#include <platform/WindowSystem.hpp>
#include <unordered_map>
#include <unordered_set>
#include <platform/window_system/glfw/WindowGLFW.hpp>

class WindowSystemGLFW : public WindowSystem
{
public:
	using CharModsCallback = void(GLFWwindow*, unsigned int, int);
	using FocusCallback = void(GLFWwindow* window, int hasFocus);
	using KeyCallback = void(GLFWwindow*, int, int, int, int);
	using MouseCallback = void(GLFWwindow* window, int button, int state, int mods);
	using ScrollCallback = void(GLFWwindow*, double xOffset, double yOffset);
	using SizeCallback = void(GLFWwindow* window, int width, int height);


	Window* createWindow(Window::WindowStruct& desc, Renderer& renderer) override;

	static WindowSystemGLFW* get();

	virtual bool init() override;

	bool isButtonDown(int glfwButton);
	bool isButtonPressed(int glfwButton);
	bool isButtonReleased(int glfwButton);
	bool isKeyDown(int glfwKey);
	bool isKeyPressed(int glfwKey);
	bool isKeyReleased(int glfwKey);

	virtual void pollEvents() override;

	Input::Button toButton(int glfwButton);
	int toGLFWbutton(Input::Button button);
	int toGLFWkey(Input::Key);
	Input::Key toKey(int glfwKey);

	void registerCharModsCallback(WindowGLFW* window, std::function<CharModsCallback> callback);

	void registerFocusCallback(WindowGLFW* window, std::function<FocusCallback> callback);

	void registerKeyCallback(WindowGLFW* window, std::function<KeyCallback> callback);

	void registerMouseCallback(WindowGLFW* window, std::function<MouseCallback> callback);

	void registerScrollCallback(WindowGLFW* window, std::function<ScrollCallback> callback);

	void registerSizeCallback(WindowGLFW* window, std::function<SizeCallback> callback);

	void removeCharModsCallback(WindowGLFW* window);

	void removeFocusCallbackCallback(WindowGLFW* window);
	void removeKeyCallback(WindowGLFW* window);
	void removeMouseCallback(WindowGLFW* window);
	void removeScrollCallback(WindowGLFW* window);
	void removeSizeCallback(WindowGLFW* window);


	virtual void terminate() override;

protected:

	static void charModsInputHandler(GLFWwindow*, unsigned int, int);

	static void errorCallback(int error, const char* description);

	static void focusInputHandler(GLFWwindow* window, int hasFocus);

	static void keyInputHandler(GLFWwindow*, int, int, int, int);

	static void mouseInputHandler(GLFWwindow* window, int button, int state, int mods);

	static void scrollInputHandler(GLFWwindow* window, double xOffset, double yOffset);

	static void sizeInputHandler(GLFWwindow* window, int width, int height);

	void initInputButtonMap();
	void initInputKeyMap();


	bool m_isInitialized;
	platform::LoggingClient logClient;

private:
	WindowSystemGLFW();

	static WindowSystemGLFW instance;

	std::list<WindowGLFW> windows;

	// callbacks
	std::unordered_map<GLFWwindow*, std::function<CharModsCallback>> charModsCallbacks;
	std::unordered_map<GLFWwindow*, std::function<FocusCallback>> focusCallbacks;
	std::unordered_map<GLFWwindow*, std::function<KeyCallback>> keyCallbacks;
	std::unordered_map<GLFWwindow*, std::function<MouseCallback>> mouseCallbacks;
	std::unordered_map<GLFWwindow*, std::function<ScrollCallback>> scrollCallbacks;
	std::unordered_map<GLFWwindow*, std::function<SizeCallback>> sizeCallbacks;

	// mapping glfw button <-> input button
	std::unordered_map<int, Input::Button> glfwToButtonMap;
	std::unordered_map<Input::Button, int> buttonToGlfwMap;

	// mapping glfw key <-> input key
	std::unordered_map<int, Input::Key> glfwToKeyMap;
	std::unordered_map<Input::Key, int> keyToGlfwMap;

	// mouse button states
	std::unordered_set<int> downButtons;
	std::unordered_set<int> pressedButtons;
	std::unordered_set<int> releasedButtons;

	// key states
	std::unordered_set<int> downKeys;
	std::unordered_set<int> pressedKeys;
	std::unordered_set<int> releasedKeys;
};