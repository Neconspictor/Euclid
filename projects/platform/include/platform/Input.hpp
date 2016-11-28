#ifndef INPUT_HPP
#define INPUT_HPP
#include <platform/logging/LoggingClient.hpp>
#include <platform/util/CallbackContainer.hpp>

class Window;

/**
 * A struct for storing/receiving relative mouse movement.
 */
struct MouseOffset
{
	int xAbsolute;
	int yAbsolute;
	int xOffset;
	int yOffset;
};

/**
 * The Input class is respsonsible for recording user key and button inputs and provides methods
 * for querying its current state. The input class is designed as a polling system and should be updated
 * each frame.
 */
class Input
{
public:

	using ScrollContainer = CallbackContainer<void(float scrollDiff)>;
	using ScrollCallback = ScrollContainer::Callback;
	using ScrollConnection = ScrollContainer::SharedItem;

	/**
	* Amount of valid buttons (without InvalidButton!!!)
	*/
	static const int BUTTON_SIZE = 3;

	/**
	*  Value of the first button
	*/
	static const int BUTTON_MIN_VALUE = 0;

	/**
	 * Defines valid buttons a user can press.
	 */
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

	/**
	* Defines valid keys a user can press.
	*/
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

	/**
	 * Stores the state of a key or button.
	 * Every input item can have one of these states.
	 */
	enum InputItemState
	{
		Up = 0,
		Pressed,
		Down,
		Released
	};

	virtual ~Input();

	/**
	* Adds a callback to scrolling events. Every time, when the user scrolls,
	* the added callback will be called.
	*/
	ScrollConnection addScrollCallback(const ScrollCallback& callback);	

	/**
	* Provides any button that was currently pressed.
	* If no valid key is pressed than Button::InvalidButton
	* will be returned.
	*/
	virtual Button getAnyPressedButton() = 0;

	/**
	 * Provides any key that was currently pressed.
	 * If no valid key is pressed than Key::InvalidKey
	 * will be returned.
	 */
	virtual Key getAnyPressedKey() = 0;

	/**
	* Returns the amount of scrolling the user did since the last frame.
	* If the result is positive, the user scrolled up, negative values means
	* down scrolling and 0 means no scolling.
	*/
	virtual float getFrameScrollOffset();

	/**
	*  Provides information about how much the cursor moved since the last frame.
	*/
	virtual MouseOffset getFrameMouseOffset();

	/**
	 * Deprectaed function. Don't use it! TODO: remove function!
	 */
	//void updateOnFrame(GLFWwindow* window, double frameTime);

	/**
	* Checks if a given input button is currently hold down.
	*/
	virtual bool isDown(Button button) = 0;

	/**
	* Checks if a given input key is currently hold down.
	*/
	virtual bool isDown(Key key) = 0;

	/**
	* Checks if a given input button is currently pressed.
	* Note: Pressing is an event that is only triggered onetime.
	* The user has to release the button again, before another
	* button press event can be triggered!
	*/
	virtual bool isPressed(Button button) = 0;

	/**
	* Checks if a given input key is currently pressed.
	* Note: Pressing is an event that is only triggered onetime.
	* The user has to release the key again, before another
	* key press event can be triggered!
	*/
	virtual bool isPressed(Key key) = 0;

	/**
	* Checks if a given input button, that was pressed or hold, is currently released.
	* Note: Releasing is an event that is only triggered onetime.
	* The user has to press the button again, before another
	* button press event can be triggered!
	*/
	virtual bool isReleased(Button button) = 0;

	/**
	* Checks if a given input key, that was pressed or hold, is currently released.
	* Note: Releasing is an event that is only triggered onetime.
	* The user has to press the key again, before another
	* key press event can be triggered!
	*/
	virtual bool isReleased(Key key) = 0;

	/**
	* An input device can be a listener for focus change events triggered
	* by a window.
	*/
	static void onWindowsFocus(Window* window, int focused);

	/**
	* Updates the input class instance, if the user scrolls on a specific window.
	*/
	static void onScroll(Window* window, double xoffset, double yoffset);

	/**
	* Removes a previously established scrolling connection. The callback of the connection
	* won't be notified anymore if scrolling events occurs.
	*/
	void removeScrollConnection(const ScrollConnection& connection);

	/**
	 * Sets the absolut mouse position in the coordination system of the current active window. 
	 */
	virtual void setMousePosition(int xPos, int yPos);

	/**
	*  Checks, if a window, this input class is listening on, is currently on focus or inactive.
	* NOTE: This input class has to be asigned as window focus listener to any window, otherwise
	* this function will always return 'false'.
	*/
	virtual bool windowHasFocus();

protected:

	/**
	* Assigns default values to its member variables.
	* Is protected as Input is an abstract class.
	*/
	Input();

	/**
	 * Copy constructor
	 */
	Input(const Input& other);

	/**
	 * The input class is a singleton. Therefore it needs one single instance.
	 */
	static Input* instance;

	int frameMouseXOffset, frameMouseYOffset;
	int mouseXabsolut, mouseYabsolut;
	float frameScrollOffset;
	bool m_windowHasFocus;
	bool firstMouseInput;

	platform::LoggingClient logClient;

	ScrollContainer scrollContainer;

	/**
	 * Calls all regsitered scrolling callbacks.
	 * This function is intended to be called when the user scrolls.
	 */
	void informScrollListeners(float scrollDiff);
};

#endif