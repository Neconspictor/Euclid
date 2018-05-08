#pragma once

#include <platform/Window.hpp>


struct GLFWwindow;
class WindowGLFW;

class InputGLFW : public Input
{
public:

	explicit InputGLFW(WindowGLFW* window);

	explicit InputGLFW(const InputGLFW& other);

	virtual ~InputGLFW();

	Button getAnyPressedButton() override;
	Key getAnyPressedKey() override;
	bool isDown(Button button) override;
	bool isDown(Key key) override;
	bool isPressed(Button button) override;
	bool isPressed(Key key) override;
	bool isReleased(Button button) override;
	bool isReleased(Key key) override;


	void scrollCallback(double xOffset, double yOffset);

	void frameUpdate();
	void resetForFrame();

	virtual void setMousePosition(int xPos, int yPos) override;
	void setWindow(WindowGLFW* window);

protected:
	WindowGLFW* window;
	Key anyPressedKey;
	Button anyPressedButton;
	platform::LoggingClient logClient;

};

class WindowGLFW : public Window
{
public:

	using CharModsCallback = void(GLFWwindow*, unsigned int, int);
	using FocusCallback = void(GLFWwindow* window, int hasFocus);
	using KeyCallback = void(GLFWwindow*, int, int, int, int);
	using MouseCallback = void(GLFWwindow* window, int button, int state, int mods);
	using RefreshCallback = void(GLFWwindow* window);
	using ScrollCallback = void(GLFWwindow*, double xOffset, double yOffset);
	using SizeCallback = void(GLFWwindow* window, int width, int height);


	WindowGLFW(WindowStruct const& description);
	virtual ~WindowGLFW();

	void activate() override;
	void close() override;
	Input* getInputDevice() override;

	GLFWwindow* getSource() const;

	virtual bool hasFocus() override;

	void init();

	bool isOpen() override;
	void minimize() override;

	void onCharMods(unsigned int codepoint, int mods);
	void onKey(int key, int scancode, int action, int mods);
	void onMouse(int button, int action, int mods);

	void registerCallbacks();

	/**
	* Releases any allocated memory
	*/
	void release();

	void registerCharModsCallback(std::function<CharModsCallback> callback);
	void registerKeyCallback(std::function<KeyCallback> callback);
	void registerMouseCallback(std::function<MouseCallback> callback);
	void registerRefreshCallback(std::function<RefreshCallback> callback);

	void removeCallbacks();

	void resize(int newWidth, int newHeight) override;
	void setCursorPosition(int xPos, int yPos) override;
	void setFocus(bool focus);
	void setFullscreen() override;

	/**
	 * Sets width and height of the window, but doesn't update the underlying GLFWwindow
	 * (-> no visual update!)
	 */
	void setSize(int width, int height);

	virtual void setTitle(const std::string& newTitle) override;

	void setVisible(bool visible) override;
	void setWindowed() override;
	void swapBuffers() override;

	static void charModsInputHandler(GLFWwindow * window, unsigned int codepoint, int mods);

	static void focusInputHandler(GLFWwindow* window, int hasFocus);

	static void keyInputHandler(GLFWwindow* window, int key, int scancode, int action, int mods);

	static void mouseInputHandler(GLFWwindow * window, int button, int action, int mods);

	static void refreshWindowHandler(GLFWwindow* window);

	static void scrollInputHandler(GLFWwindow* window, double xOffset, double yOffset);

	static void sizeInputHandler(GLFWwindow* window, int width, int height);



	bool isKeyDown(int glfwKey);
	bool isKeyPressed(int glfwKey);
	bool isKeyReleased(int glfwKey);

	bool isButtonDown(int glfwButton);
	bool isButtonPressed(int glfwButton);
	bool isButtonReleased(int glfwButton);


protected:
	void refreshWindowWithoutCallbacks();

	friend class WindowSystemGLFW;
	friend class InputGLFW;

	GLFWwindow* window;
	InputGLFW inputDevice;
	bool m_hasFocus;

	std::list<std::function<CharModsCallback>> charModsCallbacks;
	std::list<std::function<KeyCallback>> keyCallbacks;
	std::list<std::function<MouseCallback>> mouseCallbacks;
	std::list<std::function<RefreshCallback>> refreshCallbacks;


	// callbacks
	//std::list<std::function<FocusCallback>> focusCallbacks;
	std::list<std::function<ScrollCallback>> scrollCallbacks;
	std::list<std::function<SizeCallback>> sizeCallbacks;
	
	// key states
	std::unordered_set<int> downKeys;
	std::unordered_set<int> pressedKeys;
	std::unordered_set<int> releasedKeys;

	// mouse button states
	std::unordered_set<int> downButtons;
	std::unordered_set<int> pressedButtons;
	std::unordered_set<int> releasedButtons;

	bool disableCallbacks;
	
	
	void createOpenGLWindow();



};