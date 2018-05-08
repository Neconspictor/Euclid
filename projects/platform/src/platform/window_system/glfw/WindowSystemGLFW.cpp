#include <platform/window_system/glfw/WindowSystemGLFW.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <utf8.h>

using namespace std;

WindowSystemGLFW WindowSystemGLFW::instance;

std::unordered_map<int, Input::Button> WindowSystemGLFW::glfwToButtonMap;
std::unordered_map<Input::Button, int> WindowSystemGLFW::buttonToGlfwMap;

// mapping glfw key <-> input key
std::unordered_map<int, Input::Key> WindowSystemGLFW::glfwToKeyMap;
std::unordered_map<Input::Key, int> WindowSystemGLFW::keyToGlfwMap;


WindowSystemGLFW::WindowSystemGLFW() : m_isInitialized(false), logClient(platform::getLogServer())
{
}

Window* WindowSystemGLFW::createWindow(Window::WindowStruct& desc)
{
	windows.push_back(move(WindowGLFW (desc)));
	WindowGLFW* pointer = &windows.back();

	pointer->init();

	if (desc.fullscreen)
		pointer->setFullscreen();
	//else
		//pointer->setWindowed();

	
	pointer->registerCallbacks();

	return pointer;
}

Renderer* WindowSystemGLFW::getRenderer() {

	//TODO
	return nullptr;
}

Input* WindowSystemGLFW::getInput() {
	//TODO
	return nullptr;
}

void WindowSystemGLFW::errorCallback(int error, const char* description)
{
	LOG(instance.logClient, platform::Error) << "Error code: " << error
		<< ", error description: " << description;
}

WindowSystemGLFW* WindowSystemGLFW::get()
{
	return &instance;
}

bool WindowSystemGLFW::init()
{
	if (m_isInitialized) return true;

	m_isInitialized = (glfwInit() == GLFW_TRUE) ? true : false;

	if (!m_isInitialized) return false;

	glfwSetErrorCallback(errorCallback);

	initInputButtonMap();
	initInputKeyMap();
	return true;
}

int WindowSystemGLFW::toGLFWbutton(Input::Button button)
{
	auto it = buttonToGlfwMap.find(button);
	if (it == buttonToGlfwMap.end()) return GLFW_KEY_UNKNOWN;
	return it->second;
}

int WindowSystemGLFW::toGLFWkey(Input::Key key)
{
	auto it = keyToGlfwMap.find(key);
	if (it == keyToGlfwMap.end()) return GLFW_KEY_UNKNOWN;
	return it->second;
}

Input::Button WindowSystemGLFW::toButton(int glfwButton)
{
	auto it = glfwToButtonMap.find(glfwButton);
	if (it == glfwToButtonMap.end()) return Input::InvalidButton;
	return it->second;
}

Input::Key WindowSystemGLFW::toKey(int glfwKey)
{
	auto it = glfwToKeyMap.find(glfwKey);
	if (it == glfwToKeyMap.end()) return Input::KEY_UNKNOWN;
	return it->second;
}

void WindowSystemGLFW::pollEvents()
{
	for (auto& window : windows)
	{
		window.inputDevice.resetForFrame();
	}

	glfwPollEvents();

	// update input device states
	for (auto& window : windows)
	{
		window.inputDevice.frameUpdate();
	}
}

void WindowSystemGLFW::initInputKeyMap()
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

void WindowSystemGLFW::initInputButtonMap()
{
	glfwToButtonMap.insert(make_pair(GLFW_MOUSE_BUTTON_LEFT, Input::LeftMouseButton));
	glfwToButtonMap.insert(make_pair(GLFW_MOUSE_BUTTON_RIGHT, Input::RightMouseButton));
	glfwToButtonMap.insert(make_pair(GLFW_MOUSE_BUTTON_MIDDLE, Input::MiddleMouseButton));

	for (auto it = glfwToButtonMap.begin(); it != glfwToButtonMap.end(); ++it)
	{
		buttonToGlfwMap.insert(make_pair(it->second, it->first));
	}
}

void WindowSystemGLFW::terminate()
{
	if (!m_isInitialized) return;
	
	for (auto& window : windows)
	{
		window.close();
		window.release();
	}
	
	glfwTerminate();
	m_isInitialized = false;
}