#pragma once

#include <platform/Window.hpp>


struct GLFWwindow;
class WindowGLFW;


class InputMapperGLFW {
public:
	virtual ~InputMapperGLFW();

	inline Input::Button toButton(int glfwButton) const;
	inline int toGLFWbutton(Input::Button button) const;
	inline int toGLFWkey(Input::Key) const;
	inline Input::Key toKey(int glfwKey) const;

	static inline InputMapperGLFW const* get();

protected:

	InputMapperGLFW();
	void initInputButtonMap();
	void initInputKeyMap();

private:
	// mapping glfw button <-> input button
	std::unordered_map<int, Input::Button> glfwToButtonMap;
	std::unordered_map<Input::Button, int> buttonToGlfwMap;

	// mapping glfw key <-> input key
	std::unordered_map<int, Input::Key> glfwToKeyMap;
	std::unordered_map<Input::Key, int> keyToGlfwMap;

	static InputMapperGLFW instance;
};



class InputGLFW : public Input
{
public:

	using CharModsCallback = void(GLFWwindow*, unsigned int, int);
	using FocusCallback = void(GLFWwindow* window, int hasFocus);
	using KeyCallback = void(GLFWwindow*, int, int, int, int);
	using MouseCallback = void(GLFWwindow* window, int button, int state, int mods);
	using RefreshCallback = void(GLFWwindow* window);
	using ScrollCallback = void(GLFWwindow*, double xOffset, double yOffset);
	using SizeCallback = void(GLFWwindow* window, int width, int height);


	explicit InputGLFW(WindowGLFW* window);

	InputGLFW(const InputGLFW&) = delete;
	InputGLFW& operator=(const InputGLFW&) = delete;

	InputGLFW(InputGLFW&& o);
	InputGLFW& operator=(InputGLFW&& o);


	virtual ~InputGLFW();

	static void charModsInputHandler(GLFWwindow * window, unsigned int codepoint, int mods);
	static void focusInputHandler(GLFWwindow* window, int hasFocus);
	static void keyInputHandler(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void mouseInputHandler(GLFWwindow * window, int button, int action, int mods);
	static void refreshWindowHandler(GLFWwindow* window);
	static void scrollInputHandler(GLFWwindow* window, double xOffset, double yOffset);
	static void sizeInputHandler(GLFWwindow* window, int width, int height);



	inline bool areCallbacksActive() const;

	void scrollCallback(double xOffset, double yOffset);

	void disableCallbacks();
	void enableCallbacks();
	void removeCallbacks();

	void frameUpdate();

	Button getAnyPressedButton() override;
	Key getAnyPressedKey() override;
	Window* getWindow() override;
	bool isDown(Button button) override;
	bool isDown(Key key) override;
	bool isPressed(Button button) override;
	bool isPressed(Key key) override;
	bool isReleased(Button button) override;
	bool isReleased(Key key) override;


	void onCharMods(unsigned int codepoint, int mods);
	void onKey(int key, int scancode, int action, int mods);
	void onMouse(int button, int action, int mods);

	// This callbacks are primarily for classes using GLFW (e.g. GUI classes)
	// TODO: Maybe refactor it probably, so that the input base class provides this
	// functionality
	void registerCharModsCallback(std::function<CharModsCallback> callback);
	void registerKeyCallback(std::function<KeyCallback> callback);
	void registerMouseCallback(std::function<MouseCallback> callback);
	//void registerRefreshCallback(std::function<RefreshCallback> callback);

	void resetForFrame();

	virtual void setMousePosition(int xPos, int yPos) override;
	void setWindow(WindowGLFW* window);

protected:
	WindowGLFW* window;
	Key anyPressedKey;
	Button anyPressedButton;

	bool _disableCallbacks;

	platform::LoggingClient logClient;

	// key states
	std::unordered_set<int> downKeys;
	std::unordered_set<int> pressedKeys;
	std::unordered_set<int> releasedKeys;

	// mouse button states
	std::unordered_set<int> downButtons;
	std::unordered_set<int> pressedButtons;
	std::unordered_set<int> releasedButtons;


	std::list<std::function<CharModsCallback>> charModsCallbacks;
	std::list<std::function<KeyCallback>> keyCallbacks;
	std::list<std::function<MouseCallback>> mouseCallbacks;
	
	// TODO refresh callbacks are already defined in Input
	//std::list<std::function<RefreshCallback>> refreshCallbacks;


	// callbacks
	//std::list<std::function<FocusCallback>> focusCallbacks;
	//std::list<std::function<ScrollCallback>> scrollCallbacks;
	//std::list<std::function<SizeCallback>> sizeCallbacks;
};