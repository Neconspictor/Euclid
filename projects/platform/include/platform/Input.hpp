#ifndef INPUT_HPP
#define INPUT_HPP
#include "GL/glew.h"
#include <GLFW/glfw3.h>
#include <list>

class Window;

struct MouseOffset
{
	float xOffset;
	float yOffset;

	MouseOffset(float x, float y)
	{
		xOffset = x;
		yOffset = y;
	};
};


class Input
{
public:

	Input();

	/**
	* Amount of valid buttons (without InvalidButton!!!)
	*/
	static const int BUTTON_SIZE = 3;

	/**
	*  Value of the first button
	*/
	static const int BUTTON_MIN_VALUE = 0;

	// Define your user buttons
	enum Button
	{
		LeftMouseButton = BUTTON_MIN_VALUE,
		RightMouseButton,
		MiddleMouseButton,
		ScrollWheelUp,
		ScrollWheelDown,
		InvalidButton
	};

	/** 
	 * Amount of valid keys (without InvalidKey!!!)
	 */
	static const int KEY_SIZE = 10;

	/**
	*  Value of the first key
	*/
	static const int KEY_MIN_VALUE = 0;

	enum Key
	{
		// keys
		KeyW = KEY_MIN_VALUE,
		KeyA,
		KeyS,
		KeyD,
		KeyEscape,
		KeyUp,
		KeyDown,
		KeyEnter,
		KeyReturn,

		//numpad stuff
		KeyKpAdd,

		InvalidKey
	};

	enum KeyState
	{
		Up = 0,
		Pressed,
		Down,
		Released
	};

	virtual ~Input();

	virtual bool isDown(Key key) = 0;
	virtual bool isDown(Button button) = 0;

	virtual bool isPressed(Key key) = 0;
	virtual bool isPressed(Button button) = 0;

	virtual bool isReleased(Key key) = 0;
	virtual bool isReleased(Button button) = 0;

	virtual Key getAnyPressedKey() = 0;
	virtual Button getAnyPressedButton() = 0;

	static void onWindowsFocus(Window* window, int focused);
	static void onScroll(Window* window, double xoffset, double yoffset);


	void updateOnFrame(GLFWwindow* window, double frameTime);

	virtual MouseOffset getFrameMouseOffset();

	virtual bool windowHasFocus();

	virtual float getFrameScrollOffset();

protected:
	Input(const Input& other);

	static Input* instance;

	float frameMouseXOffset, frameMouseYOffset;
	float mouseXabsolut, mouseYabsolut;
	float frameScrollOffset;
	bool m_windowHasFocus;
	bool firstMouseInput;
};

#endif