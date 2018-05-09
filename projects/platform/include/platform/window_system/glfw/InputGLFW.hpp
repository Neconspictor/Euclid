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