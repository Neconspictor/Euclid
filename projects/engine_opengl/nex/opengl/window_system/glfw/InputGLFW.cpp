// glad has to be included before glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <nex/opengl/window_system/glfw/InputGLFW.hpp>
#include <nex/opengl/window_system/glfw/WindowGLFW.hpp>
#include <nex/opengl/window_system/glfw/SubSystemProviderGLFW.hpp>
#include <functional>


using namespace std;


nex::InputMapperGLFW nex::InputMapperGLFW::instance;

int nex::InputMapperGLFW::toGLFWbutton(Input::Button button) const
{
	auto it = buttonToGlfwMap.find(button);
	if (it == buttonToGlfwMap.end()) return GLFW_KEY_UNKNOWN;
	return it->second;
}

int nex::InputMapperGLFW::toGLFWkey(Input::Key key) const
{
	auto it = keyToGlfwMap.find(key);
	if (it == keyToGlfwMap.end()) return GLFW_KEY_UNKNOWN;
	return it->second;
}

nex::InputMapperGLFW::InputMapperGLFW()
{
	initInputButtonMap();
	initInputKeyMap();
}


Input::Button nex::InputMapperGLFW::toButton(int glfwButton) const
{
	auto it = glfwToButtonMap.find(glfwButton);
	if (it == glfwToButtonMap.end()) return Input::InvalidButton;
	return it->second;
}

Input::Key nex::InputMapperGLFW::toKey(int glfwKey) const
{
	auto it = glfwToKeyMap.find(glfwKey);
	if (it == glfwToKeyMap.end()) return Input::KEY_UNKNOWN;
	return it->second;
}

nex::InputMapperGLFW const* nex::InputMapperGLFW::get()
{
	return &instance;
}

void nex::InputMapperGLFW::initInputButtonMap()
{
	glfwToButtonMap.insert(make_pair(GLFW_MOUSE_BUTTON_LEFT, Input::LeftMouseButton));
	glfwToButtonMap.insert(make_pair(GLFW_MOUSE_BUTTON_RIGHT, Input::RightMouseButton));
	glfwToButtonMap.insert(make_pair(GLFW_MOUSE_BUTTON_MIDDLE, Input::MiddleMouseButton));

	for (auto it = glfwToButtonMap.begin(); it != glfwToButtonMap.end(); ++it)
	{
		buttonToGlfwMap.insert(make_pair(it->second, it->first));
	}
}

void nex::InputMapperGLFW::initInputKeyMap()
{
	glfwToKeyMap.insert(make_pair(GLFW_KEY_UNKNOWN, Input::Key::KEY_UNKNOWN));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_SPACE, Input::Key::KEY_SPACE));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_APOSTROPHE, Input::Key::KEY_APOSTROPHE));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_COMMA, Input::Key::KEY_COMMA));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_MINUS, Input::Key::KEY_MINUS));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_PERIOD, Input::Key::KEY_PERIOD));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_SLASH, Input::Key::KEY_SLASH));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_0, Input::Key::KEY_0));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_1, Input::Key::KEY_1));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_2, Input::Key::KEY_2));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_3, Input::Key::KEY_3));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_4, Input::Key::KEY_4));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_5, Input::Key::KEY_5));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_6, Input::Key::KEY_6));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_7, Input::Key::KEY_7));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_8, Input::Key::KEY_8));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_9, Input::Key::KEY_9));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_SEMICOLON, Input::Key::KEY_SEMICOLON));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_EQUAL, Input::Key::KEY_EQUALS));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_A, Input::Key::KEY_A));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_B, Input::Key::KEY_B));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_C, Input::Key::KEY_C));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_D, Input::Key::KEY_D));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_E, Input::Key::KEY_E));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F, Input::Key::KEY_F));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_G, Input::Key::KEY_G));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_H, Input::Key::KEY_H));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_I, Input::Key::KEY_I));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_J, Input::Key::KEY_J));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_K, Input::Key::KEY_K));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_L, Input::Key::KEY_L));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_M, Input::Key::KEY_M));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_N, Input::Key::KEY_N));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_O, Input::Key::KEY_O));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_P, Input::Key::KEY_P));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_Q, Input::Key::KEY_Q));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_R, Input::Key::KEY_R));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_S, Input::Key::KEY_S));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_T, Input::Key::KEY_T));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_U, Input::Key::KEY_U));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_V, Input::Key::KEY_V));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_W, Input::Key::KEY_W));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_X, Input::Key::KEY_X));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_Y, Input::Key::KEY_Y));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_Z, Input::Key::KEY_Z));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_LEFT_BRACKET, Input::Key::KEY_LEFTBRACKET));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_BACKSLASH, Input::Key::KEY_BACKSLASH));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_RIGHT_BRACKET, Input::Key::KEY_RIGHTBRACKET));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_GRAVE_ACCENT, Input::Key::KEY_GRAVE));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_WORLD_1, Input::Key::KEY_NONUSHASH));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_WORLD_2, Input::Key::KEY_NONUSBACKSLASH));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_ESCAPE, Input::Key::KEY_ESCAPE));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_ENTER, Input::Key::KEY_RETURN));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_TAB, Input::Key::KEY_TAB));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_BACKSPACE, Input::Key::KEY_BACKSPACE));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_INSERT, Input::Key::KEY_INSERT));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_DELETE, Input::Key::KEY_DELETE));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_RIGHT, Input::Key::KEY_RIGHT));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_LEFT, Input::Key::KEY_LEFT));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_DOWN, Input::Key::KEY_DOWN));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_UP, Input::Key::KEY_UP));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_PAGE_UP, Input::Key::KEY_PAGEUP));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_PAGE_DOWN, Input::Key::KEY_PAGEDOWN));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_HOME, Input::Key::KEY_HOME));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_END, Input::Key::KEY_END));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_CAPS_LOCK, Input::Key::KEY_CAPSLOCK));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_SCROLL_LOCK, Input::Key::KEY_CAPSLOCK));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_NUM_LOCK, Input::Key::KEY_CAPSLOCK));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_PRINT_SCREEN, Input::Key::KEY_CAPSLOCK));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_PAUSE, Input::Key::KEY_CAPSLOCK));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F1, Input::Key::KEY_F1));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F2, Input::Key::KEY_F2));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F3, Input::Key::KEY_F3));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F4, Input::Key::KEY_F4));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F5, Input::Key::KEY_F5));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F6, Input::Key::KEY_F6));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F7, Input::Key::KEY_F7));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F8, Input::Key::KEY_F8));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F9, Input::Key::KEY_F9));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F10, Input::Key::KEY_F10));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F11, Input::Key::KEY_F11));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F12, Input::Key::KEY_F12));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F13, Input::Key::KEY_F13));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F14, Input::Key::KEY_F14));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F15, Input::Key::KEY_F15));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F16, Input::Key::KEY_F16));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F17, Input::Key::KEY_F17));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F18, Input::Key::KEY_F18));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F19, Input::Key::KEY_F19));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F20, Input::Key::KEY_F20));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F21, Input::Key::KEY_F21));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F22, Input::Key::KEY_F22));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F23, Input::Key::KEY_F23));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F24, Input::Key::KEY_F24));

	// F25 not supported
	glfwToKeyMap.insert(make_pair(GLFW_KEY_F25, Input::Key::KEY_UNKNOWN));

	glfwToKeyMap.insert(make_pair(GLFW_KEY_KP_0, Input::Key::KEY_KP_0));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_KP_1, Input::Key::KEY_KP_1));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_KP_2, Input::Key::KEY_KP_2));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_KP_3, Input::Key::KEY_KP_3));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_KP_4, Input::Key::KEY_KP_4));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_KP_5, Input::Key::KEY_KP_5));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_KP_6, Input::Key::KEY_KP_6));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_KP_7, Input::Key::KEY_KP_7));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_KP_8, Input::Key::KEY_KP_8));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_KP_9, Input::Key::KEY_KP_9));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_KP_DECIMAL, Input::Key::KEY_KP_DECIMAL));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_KP_DIVIDE, Input::Key::KEY_KP_DIVIDE));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_KP_MULTIPLY, Input::Key::KEY_KP_MULTIPLY));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_KP_SUBTRACT, Input::Key::KEY_KP_MINUS));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_KP_ADD, Input::Key::KEY_KP_PLUS));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_KP_ENTER, Input::Key::KEY_KP_ENTER));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_KP_EQUAL, Input::Key::KEY_KP_EQUALS));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_LEFT_SHIFT, Input::Key::KEY_LEFT_SHIFT));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_LEFT_CONTROL, Input::Key::KEY_LEFT_CONTROL));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_LEFT_ALT, Input::Key::KEY_LEFT_ALT));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_LEFT_SUPER, Input::Key::KEY_LEFT_SUPER));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_RIGHT_SHIFT, Input::Key::KEY_RIGHT_SHIFT));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_RIGHT_CONTROL, Input::Key::KEY_RIGHT_CONTROL));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_RIGHT_ALT, Input::Key::KEY_RIGHT_ALT));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_RIGHT_SUPER, Input::Key::KEY_RIGHT_SUPER));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_MENU, Input::Key::KEY_MENU));
	glfwToKeyMap.insert(make_pair(GLFW_KEY_LAST, Input::Key::NUM_KEYS));

	// now fill key to glfw map with the data in glfwToKeyMap
	for (auto it = glfwToKeyMap.begin(); it != glfwToKeyMap.end(); ++it)
	{
		keyToGlfwMap.insert(make_pair(it->second, it->first));
	}
}



nex::InputGLFW::InputGLFW(nex::WindowGLFW* window) : Input(), window(window),
anyPressedKey(KEY_UNKNOWN), anyPressedButton(InvalidButton), _disableCallbacks(false), m_logger("InputGLFW")
{
}

nex::InputGLFW::InputGLFW(nex::InputGLFW && o) : Input(move(o)), m_logger(move(o.m_logger))
{
	window = o.window;
	anyPressedKey = o.anyPressedKey;
	anyPressedButton = o.anyPressedButton;
	_disableCallbacks = o._disableCallbacks;

	downKeys = move(o.downKeys);
	pressedKeys = move(o.pressedKeys);
	releasedKeys = move(o.releasedKeys);

	downButtons = move(o.downButtons);
	pressedButtons = move(o.pressedButtons);
	releasedButtons = move(o.releasedButtons);

	charModsCallbacks = move(o.charModsCallbacks);
	keyCallbacks = move(o.keyCallbacks);
	mouseCallbacks = move(o.mouseCallbacks);

	//refreshCallbacks = move(o.refreshCallbacks);

	//o.window = nullptr;
}

nex::InputGLFW & nex::InputGLFW::operator=(nex::InputGLFW && o)
{
	if (this == &o) return *this;

	window = o.window;
	anyPressedKey = o.anyPressedKey;
	anyPressedButton = o.anyPressedButton;
	_disableCallbacks = o._disableCallbacks;

	downKeys = move(o.downKeys);
	pressedKeys = move(o.pressedKeys);
	releasedKeys = move(o.releasedKeys);

	downButtons = move(o.downButtons);
	pressedButtons = move(o.pressedButtons);
	releasedButtons = move(o.releasedButtons);

	charModsCallbacks = move(o.charModsCallbacks);
	keyCallbacks = move(o.keyCallbacks);
	mouseCallbacks = move(o.mouseCallbacks);

	//refreshCallbacks = move(o.refreshCallbacks);

	o.window = nullptr;

	return *this;
}

void nex::InputGLFW::charModsInputHandler(GLFWwindow * window, unsigned int codepoint, int mods)
{
	/*vector<unsigned char> utf8Result;

	utf8::utf32to8(&codepoint, &codepoint + 1, back_inserter(utf8Result));

	stringstream ss;
	for (char c : utf8Result)
	{
	ss << c;
	}

	cout << " WindowGLFW::charModsInputHandler(): " << ss.str() << endl;*/

	InputGLFW* input = static_cast<InputGLFW*>(glfwGetWindowUserPointer(window));
	if (input == nullptr || !input->areCallbacksActive()) return;

	input->onCharMods(codepoint, mods);
}

void nex::InputGLFW::closeWindowCallbackHandler(GLFWwindow* window)
{
	InputGLFW* input = static_cast<InputGLFW*>(glfwGetWindowUserPointer(window));
	if (input == nullptr || !input->areCallbacksActive()) return;
	input->informWindowCloseListeners();
}

void nex::InputGLFW::focusInputHandler(GLFWwindow * window, int hasFocus)
{
	InputGLFW* input = static_cast<InputGLFW*>(glfwGetWindowUserPointer(window));
	if (input == nullptr || !input->areCallbacksActive()) return;

	bool focus = hasFocus == GLFW_TRUE ? true : false;

	assert(input->window != nullptr);

	input->window->setFocus(focus);
}

void nex::InputGLFW::keyInputHandler(GLFWwindow* window, int key, int scancode, int action, int mods)
{

	InputGLFW* input = static_cast<InputGLFW*>(glfwGetWindowUserPointer(window));
	if (input == nullptr || !input->areCallbacksActive()) return;

	input->onKey(key, scancode, action, mods);
}

void nex::InputGLFW::mouseInputHandler(GLFWwindow * window, int button, int action, int mods)
{
	InputGLFW* input = static_cast<InputGLFW*>(glfwGetWindowUserPointer(window));
	if (input == nullptr || !input->areCallbacksActive()) return;

	input->onMouse(button, action, mods);
}

void nex::InputGLFW::scrollInputHandler(GLFWwindow * window, double xOffset, double yOffset)
{
	//TODO refactor
	InputGLFW* input = static_cast<InputGLFW*>(glfwGetWindowUserPointer(window));
	if (input == nullptr || !input->areCallbacksActive()) return;

	input->scrollCallback(xOffset, yOffset);
}

void nex::InputGLFW::sizeInputHandler(GLFWwindow * window, int width, int height)
{
	InputGLFW* input = static_cast<InputGLFW*>(glfwGetWindowUserPointer(window));
	if (input == nullptr || !input->areCallbacksActive()) return;

	assert(input->window != nullptr);

	input->window->setSize(width, height);
}


inline bool nex::InputGLFW::areCallbacksActive() const
{
	return !_disableCallbacks;
}

void nex::InputGLFW::scrollCallback(double xOffset, double yOffset)
{
	frameScrollOffsetX = xOffset;
	frameScrollOffsetY = yOffset;

	if (frameScrollOffsetX || frameScrollOffsetY)
	{
		informScrollListeners(xOffset, yOffset);
	}
}

void nex::InputGLFW::disableCallbacks()
{
	_disableCallbacks = true;
}
void nex::InputGLFW::enableCallbacks()
{
	using namespace placeholders;

	//setWindow(this); TODO

	glfwSetCharModsCallback(window->getSource(), charModsInputHandler);

	glfwSetWindowCloseCallback(window->getSource(), closeWindowCallbackHandler);

	glfwSetWindowFocusCallback(window->getSource(), focusInputHandler);

	glfwSetKeyCallback(window->getSource(), keyInputHandler);

	glfwSetMouseButtonCallback(window->getSource(), mouseInputHandler);

	glfwSetWindowSizeCallback(window->getSource(), sizeInputHandler);

	glfwSetScrollCallback(window->getSource(), scrollInputHandler);

	glfwSetWindowUserPointer(window->getSource(), this);

	_disableCallbacks = false;
}

void nex::InputGLFW::removeCallbacks()
{
	glfwSetCharModsCallback(window->getSource(), nullptr);

	glfwSetWindowCloseCallback(window->getSource(), nullptr);

	glfwSetWindowFocusCallback(window->getSource(), nullptr);

	glfwSetKeyCallback(window->getSource(), nullptr);

	glfwSetMouseButtonCallback(window->getSource(), nullptr);

	glfwSetWindowRefreshCallback(window->getSource(), nullptr);

	glfwSetWindowSizeCallback(window->getSource(), nullptr);

	glfwSetScrollCallback(window->getSource(), nullptr);
}

void nex::InputGLFW::frameUpdate()
{
	double mouseX, mouseY;
	glfwGetCursorPos(window->getSource(), &mouseX, &mouseY);
	int mouseXOld = mouseXabsolut;
	int mouseYOld = mouseYabsolut;
	mouseXabsolut = static_cast<int>(mouseX);
	mouseYabsolut = static_cast<int>(mouseY);

	frameMouseXOffset = mouseXabsolut - mouseXOld;
	frameMouseYOffset = mouseYabsolut - mouseYOld;

	if (!isDown(anyPressedKey))
	{
		anyPressedKey = KEY_UNKNOWN;
	}

	if (!isDown(anyPressedButton))
	{
		anyPressedButton = InvalidButton;
	}
}

Input::Button nex::InputGLFW::getAnyPressedButton()
{
	return anyPressedButton;
}

Input::Key nex::InputGLFW::getAnyPressedKey()
{
	return anyPressedKey;
}

Window * nex::InputGLFW::getWindow()
{
	return window;
}

bool nex::InputGLFW::isDown(Button button)
{
	int glfwButton = InputMapperGLFW::get()->toGLFWbutton(button);
	if (glfwButton == GLFW_KEY_UNKNOWN) return false;
	return glfwGetMouseButton(window->getSource(), glfwButton) != GLFW_RELEASE;
}

bool nex::InputGLFW::isDown(Key key)
{
	int glfwKey = InputMapperGLFW::get()->toGLFWkey(key);
	if (glfwKey == GLFW_KEY_UNKNOWN) return false;
	return glfwGetKey(window->getSource(), glfwKey) != GLFW_RELEASE;
}

bool nex::InputGLFW::isPressed(Button button)
{
	int glfwButton = InputMapperGLFW::get()->toGLFWbutton(button);
	if (glfwButton == GLFW_KEY_UNKNOWN) return false;
	return pressedButtons.find(glfwButton) != pressedButtons.end();
}

bool nex::InputGLFW::isPressed(Key key)
{
	int glfwKey = InputMapperGLFW::get()->toGLFWkey(key);
	if (glfwKey == GLFW_KEY_UNKNOWN) return false;
	return pressedKeys.find(glfwKey) != pressedKeys.end();
}

bool nex::InputGLFW::isReleased(Button button)
{
	int glfwButton = InputMapperGLFW::get()->toGLFWbutton(button);
	if (glfwButton == GLFW_KEY_UNKNOWN) return false;
	return releasedButtons.find(glfwButton) != releasedButtons.end();
}

bool nex::InputGLFW::isReleased(Key key)
{
	int glfwKey = InputMapperGLFW::get()->toGLFWkey(key);
	if (glfwKey == GLFW_KEY_UNKNOWN) return false;
	return releasedKeys.find(glfwKey) != releasedKeys.end();
}

void nex::InputGLFW::onCharMods(unsigned int codepoint, int mods)
{
	for (auto& c : charModsCallbacks)
	{
		c(window->getSource(), codepoint, mods);
	}
}

void nex::InputGLFW::onKey(int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (pressedKeys.find(key) == pressedKeys.end())
		{
			pressedKeys.insert(key);
		}

		if (releasedKeys.find(key) != releasedKeys.end())
		{
			releasedKeys.erase(releasedKeys.find(key));
		}
	}

	if (action == GLFW_RELEASE)
	{
		if (releasedKeys.find(key) == releasedKeys.end())
		{
			releasedKeys.insert(key);
		}

		if (pressedKeys.find(key) != pressedKeys.end())
		{
			pressedKeys.erase(pressedKeys.find(key));
		}
	}


	for (auto& c : keyCallbacks)
	{
		c(window->getSource(), key, scancode, action, mods);
	}
}

void nex::InputGLFW::onMouse(int button, int action, int mods)
{

	if (action == GLFW_PRESS)
	{
		if (pressedButtons.find(button) == pressedButtons.end())
		{
			pressedButtons.insert(button);
		}

		if (releasedButtons.find(button) != releasedButtons.end())
		{
			releasedButtons.erase(releasedButtons.find(button));
		}
	}

	if (action == GLFW_RELEASE)
	{
		if (releasedButtons.find(button) == releasedButtons.end())
		{
			releasedButtons.insert(button);
		}

		if (pressedButtons.find(button) != pressedButtons.end())
		{
			pressedButtons.erase(pressedButtons.find(button));
		}
	}

	for (auto& c : mouseCallbacks)
	{
		c(window->getSource(), button, action, mods);
	}
}

void nex::InputGLFW::registerCharModsCallback(function<CharModsCallback> callback)
{
	charModsCallbacks.push_back(callback);
}

void nex::InputGLFW::registerKeyCallback(function<KeyCallback> callback)
{
	keyCallbacks.push_back(callback);
}

void nex::InputGLFW::registerMouseCallback(function<MouseCallback> callback)
{
	mouseCallbacks.push_back(callback);
}

void nex::InputGLFW::resetForFrame()
{
	// clear state of keys  and mouse buttons before polling, as callbacks are called during polling!
	// TODO move releasedKeys/pressedKeys to this class
	releasedKeys.clear();
	pressedKeys.clear();
	releasedButtons.clear();
	pressedButtons.clear();

	frameScrollOffsetX = 0;
	frameScrollOffsetY = 0;
}

void nex::InputGLFW::setMousePosition(int xPos, int yPos)
{
	glfwSetCursorPos(window->getSource(), static_cast<double>(xPos), static_cast<double>(yPos));
	Input::setMousePosition(xPos, yPos);
}

void nex::InputGLFW::setWindow(WindowGLFW* window)
{
	this->window = window;
}