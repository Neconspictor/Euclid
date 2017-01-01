#pragma once

#include <platform/Window.hpp>


struct GLFWwindow;
class WindowGLFW;

class InputGLFW : public Input
{
public:

	explicit InputGLFW(WindowGLFW* window);

	explicit InputGLFW(const InputGLFW& other);

	Button getAnyPressedButton() override;
	Key getAnyPressedKey() override;
	bool isDown(Button button) override;
	bool isDown(Key key) override;
	bool isPressed(Button button) override;
	bool isPressed(Key key) override;
	bool isReleased(Button button) override;
	bool isReleased(Key key) override;

	void charModsCallback(GLFWwindow* window, unsigned int codepoint, int mods);
	void focusCallback(GLFWwindow* window, int hasFocus);
	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void mouseCallback(GLFWwindow* window, int button, int action, int mods);
	void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
	void sizeCallback(GLFWwindow* window, int width, int height);

	void frameUpdate();
	void resetForFrame();

	virtual void setMousePosition(int xPos, int yPos) override;
	void setWindow(WindowGLFW* window);

protected:
	WindowGLFW* window;
	platform::LoggingClient logClient;
};

class WindowGLFW : public Window
{
public:

	using CharModsCallback = void(GLFWwindow*, unsigned int, int);
	using FocusCallback = void(GLFWwindow* window, int hasFocus);
	using KeyCallback = void(GLFWwindow*, int, int, int, int);
	using MouseCallback = void(GLFWwindow* window, int button, int state, int mods);
	using ScrollCallback = void(GLFWwindow*, double xOffset, double yOffset);
	using SizeCallback = void(GLFWwindow* window, int width, int height);

	WindowGLFW(WindowStruct const& description, Renderer& renderer);
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
	void onMouse(int button, int state, int mods);

	void registerCallbacks();

	/**
	* Releases any allocated memory
	*/
	void release();

	void registerCharModsCallback(std::function<CharModsCallback> callback);
	void registerKeyCallback(std::function<KeyCallback> callback);
	void registerMouseCallback(std::function<MouseCallback> callback);

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
protected:

	friend class WindowSystemGLFW;

	GLFWwindow* window;
	InputGLFW inputDevice;
	bool m_hasFocus;

	std::list<std::function<CharModsCallback>> charModsCallbacks;
	std::list<std::function<KeyCallback>> keyCallbacks;
	std::list<std::function<MouseCallback>> mouseCallbacks;

	void createNoAPIWindow();
	void createOpenGLWindow();


};