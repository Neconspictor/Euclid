// glad has to be included before glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <platform/window_system/glfw/WindowGLFW.hpp>
#include <platform/window_system/glfw/WindowSystemGLFW.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <functional>


using namespace std;
using namespace platform;


WindowGLFW::WindowGLFW(WindowStruct const& desc) :
	Window(desc), window(nullptr), inputDevice(this), m_hasFocus(true), disableCallbacks(false)
{
}

WindowGLFW::~WindowGLFW()
{
}

void WindowGLFW::activate()
{
	glfwMakeContextCurrent(window);
}

void WindowGLFW::close()
{
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

Input* WindowGLFW::getInputDevice()
{
	return &inputDevice;
}

GLFWwindow* WindowGLFW::getSource() const
{
	return window;
}

bool WindowGLFW::hasFocus()
{
	return m_hasFocus;
}

void WindowGLFW::init()
{
	if (!WindowSystemGLFW::get()->init())
	{
		throw runtime_error("WindowGLFW: Error: Couldn't initialize GLFWSystem!");
	}

	createOpenGLWindow();
}

bool WindowGLFW::isOpen()
{
	m_isOpen = glfwWindowShouldClose(window) ? false : true;
	return m_isOpen;
}

void WindowGLFW::minimize()
{
	glfwIconifyWindow(window);
}

void WindowGLFW::onCharMods(unsigned int codepoint, int mods)
{
	for (auto& c : charModsCallbacks)
	{
		c(this->window, codepoint, mods);
	}
}

void WindowGLFW::onKey(int key, int scancode, int action, int mods)
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
		c(this->window, key, scancode, action, mods);
	}
}

void WindowGLFW::onMouse(int button, int action, int mods)
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
		c(this->window, button, action, mods);
	}
}

void WindowGLFW::registerCallbacks()
{
	using namespace placeholders;

	inputDevice.setWindow(this);

	glfwSetCharModsCallback(window, &charModsInputHandler);

	glfwSetWindowFocusCallback(window, focusInputHandler);

	glfwSetKeyCallback(window, keyInputHandler);

	glfwSetMouseButtonCallback(window, mouseInputHandler);

	glfwSetWindowRefreshCallback(window, refreshWindowHandler);

	glfwSetWindowSizeCallback(window, sizeInputHandler);

	glfwSetScrollCallback(window, scrollInputHandler);

	glfwSetWindowUserPointer(window, this);

	disableCallbacks = false;
}

void WindowGLFW::release()
{
	glfwDestroyWindow(window);
	window = nullptr;
}

void WindowGLFW::registerCharModsCallback(function<CharModsCallback> callback)
{
	charModsCallbacks.push_back(callback);
}

void WindowGLFW::registerKeyCallback(function<KeyCallback> callback)
{
	keyCallbacks.push_back(callback);
}

void WindowGLFW::registerMouseCallback(function<MouseCallback> callback)
{
	mouseCallbacks.push_back(callback);
}

void WindowGLFW::registerRefreshCallback(std::function<RefreshCallback> callback)
{
	refreshCallbacks.push_back(callback);
}

void WindowGLFW::removeCallbacks()
{
	disableCallbacks = true;
}

void WindowGLFW::resize(int newWidth, int newHeight)
{
	width = newWidth;
	height = newHeight;
	glfwSetWindowSize(window, width, height);
	informResizeListeners(width, height);
}

void WindowGLFW::setCursorPosition(int xPos, int yPos)
{
	inputDevice.setMousePosition(xPos, yPos);
}

void WindowGLFW::setFocus(bool focus)
{
	if (focus) {
		LOG(logClient, platform::Debug) << "gained focus!";
	} else {
		LOG(logClient, platform::Debug) << "lost focus!";
	}

	m_hasFocus = focus;
	informWindowFocusListeners(focus);
}

void WindowGLFW::setFullscreen()
{
	glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), this->posX, this->posY, this->width, this->height, this->refreshRate);
	fullscreen = true;
}

void WindowGLFW::setSize(int width, int height)
{
	this->width = width;
	this->height = height;
	informResizeListeners(width, height);
}

void WindowGLFW::setTitle(const string& newTitle)
{
	Window::setTitle(newTitle);
	glfwSetWindowTitle(window, title.c_str());
}

void WindowGLFW::setVisible(bool visible)
{
	if (m_isVisible == visible) return;

	m_isVisible = visible;

	if (m_isVisible)
		glfwShowWindow(window);
	else
		glfwHideWindow(window);
}

void WindowGLFW::setWindowed()
{	
	glfwSetWindowMonitor(window, nullptr, this->posX, this->posY, this->width, this->height, this->refreshRate);
	
	int top, left, bottom, right;
	glfwGetWindowFrameSize(window, &left, &top, &right, &bottom);
	glfwSetWindowPos(window, this->posX + left, this->posY + top);

	fullscreen = false;

	refreshWindowWithoutCallbacks();

	//now inform listeners
	//setSize(width, height);
	//informResizeListeners(width, height);
}

void WindowGLFW::swapBuffers()
{
	//glViewport(0, 0, 800, 600);
	//glBindSampler(0, 0);
	glfwSwapBuffers(window);
}

void WindowGLFW::charModsInputHandler(GLFWwindow * window, unsigned int codepoint, int mods)
{
	/*vector<unsigned char> utf8Result;

	utf8::utf32to8(&codepoint, &codepoint + 1, back_inserter(utf8Result));

	stringstream ss;
	for (char c : utf8Result)
	{
		ss << c;
	}

	cout << " WindowGLFW::charModsInputHandler(): " << ss.str() << endl;*/

	WindowGLFW* windowGLFW = static_cast<WindowGLFW*>(glfwGetWindowUserPointer(window));

	if (windowGLFW == nullptr || windowGLFW->disableCallbacks) return;

	windowGLFW->onCharMods(codepoint, mods);
}

void WindowGLFW::focusInputHandler(GLFWwindow * window, int hasFocus)
{
	WindowGLFW* windowGLFW = static_cast<WindowGLFW*>(glfwGetWindowUserPointer(window));

	if (windowGLFW == nullptr || windowGLFW->disableCallbacks) return;
	
	bool focus = hasFocus == GLFW_TRUE ? true : false;
	windowGLFW->setFocus(focus);
}

void WindowGLFW::keyInputHandler(GLFWwindow* window, int key, int scancode, int action, int mods)
{

	WindowGLFW* windowGLFW = static_cast<WindowGLFW*>(glfwGetWindowUserPointer(window));

	if (windowGLFW == nullptr || windowGLFW->disableCallbacks) return;
	
	windowGLFW->onKey(key, scancode, action, mods);
}

void WindowGLFW::mouseInputHandler(GLFWwindow * window, int button, int action, int mods)
{
	WindowGLFW* windowGLFW = static_cast<WindowGLFW*>(glfwGetWindowUserPointer(window));

	if (windowGLFW == nullptr || windowGLFW->disableCallbacks) return;

	windowGLFW->onMouse(button, action, mods);
}

void WindowGLFW::refreshWindowHandler(GLFWwindow * window)
{
	WindowGLFW* windowGLFW = static_cast<WindowGLFW*>(glfwGetWindowUserPointer(window));

	if (windowGLFW == nullptr ||  windowGLFW->disableCallbacks) return;

	windowGLFW->informRefreshListeners();
}

void WindowGLFW::scrollInputHandler(GLFWwindow * window, double xOffset, double yOffset)
{
	//TODO refactor
	WindowGLFW* windowGLFW = static_cast<WindowGLFW*>(glfwGetWindowUserPointer(window));

	if (windowGLFW == nullptr || windowGLFW->disableCallbacks) return;

	InputGLFW* inputGLFW = static_cast<InputGLFW*>(windowGLFW->getInputDevice());
	inputGLFW->scrollCallback(xOffset, yOffset);
}

void WindowGLFW::sizeInputHandler(GLFWwindow * window, int width, int height)
{
	WindowGLFW* windowGLFW = static_cast<WindowGLFW*>(glfwGetWindowUserPointer(window));

	if (windowGLFW == nullptr || windowGLFW->disableCallbacks) return;

	windowGLFW->setSize(width, height);
}


bool WindowGLFW::isKeyDown(int glfwKey)
{
	return downKeys.find(glfwKey) != downKeys.end();
}

bool WindowGLFW::isKeyPressed(int glfwKey)
{
	return pressedKeys.find(glfwKey) != pressedKeys.end();
}

bool WindowGLFW::isKeyReleased(int glfwKey)
{
	return releasedKeys.find(glfwKey) != releasedKeys.end();
}

bool WindowGLFW::isButtonDown(int glfwButton)
{
	return downButtons.find(glfwButton) != downButtons.end();
}

bool WindowGLFW::isButtonPressed(int glfwButton)
{
	return pressedButtons.find(glfwButton) != pressedButtons.end();
}

bool WindowGLFW::isButtonReleased(int glfwButton)
{
	return releasedButtons.find(glfwButton) != releasedButtons.end();
}


void WindowGLFW::refreshWindowWithoutCallbacks()
{

	// assure that o callbacks get called!
	removeCallbacks();
	//WindowSystemGLFW* system = WindowSystemGLFW::get();
	//system->removeSizeCallback(this);

	//register a dummy callback that does nothing
	//system->registerRefreshCallback(this, [](GLFWwindow*){}); 

	// change the size of the window temporarily in order to trigger the operating system to repaint the window
	// To avoid issues we don't propagate this temporary change
	int widthBackup = width;

	glfwSetWindowSize(window, 0, height);

	int top, left, bottom, right;
	glfwGetWindowFrameSize(window, &left, &top, &right, &bottom);

	int newWidth = right - left;

	// nothing has changed
	if (newWidth == widthBackup) {
		glfwSetWindowSize(window, newWidth+1, height);
	}

	// hack!!! so that opengl recognises that it should update 
	// TODO: Do not use opengl commands in the window class!
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	glfwSwapBuffers(window);

	//now restore the original window size
	glfwSetWindowSize(window, widthBackup, height);

	// restore the callbacks
	registerCallbacks();
}

void WindowGLFW::createOpenGLWindow()
{
	//TODO
	/*glfwWindowHint(GLFW_VISIBLE, m_isVisible ? GLFW_TRUE : GLFW_FALSE);*/

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_RED_BITS, 32);
	glfwWindowHint(GLFW_GREEN_BITS, 32);
	glfwWindowHint(GLFW_BLUE_BITS, 32);
	glfwWindowHint(GLFW_ALPHA_BITS, 32);
	glfwWindowHint(GLFW_STENCIL_BITS, 32);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);


	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 16);
	glfwWindowHint(GLFW_REFRESH_RATE, this->refreshRate);

	//glfwWindowHint(GLFW_DECORATED, GL_FALSE);
	//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


	window = glfwCreateWindow(this->width, this->height, this->title.c_str(), nullptr, nullptr);

	if (!window)
	{
		throw runtime_error("WindowGLFW: Error: Couldn't create GLFWwindow!");
	}

	int top, left, bottom, right;
	//glfwGetWindowFrameSize(window, &left, &top, &right, &bottom);

	//glfwSetWindowPos(window, this->posX + left, this->posY + top);

	glfwMakeContextCurrent(window);
	//gladLoadGLLoader(GLADloadproc(glfwGetProcAddress));

//#if defined(NANOGUI_GLAD)
	//if (!gladLoadGL())
	//if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		//throw std::runtime_error("Could not initialize GLAD!");
	//glGetError(); // pull and ignore unhandled errors like GL_INVALID_ENUM
//#endif

	// Load all OpenGL functions using the glfw loader function
	// If you use SDL you can use: https://wiki.libsdl.org/SDL_GL_GetProcAddress
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	//if (!gladLoadGL())
	{
		throw runtime_error("WindowGLFW::createOpenGLWindow(): Failed to initialize OpenGL context");
	}
	/*if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
		throw runtime_error("WindowGLFW::createOpenGLWindow(): Failed to initialize OpenGL context");
	}*/

	if (!vSync)
	{
		glfwSwapInterval(0);
	} else
	{
		glfwSwapInterval(1);
	}

	// Alternative use the builtin loader, e.g. if no other loader function is available
	/*
	if (!gladLoadGL()) {
	std::cout << "Failed to initialize OpenGL context" << std::endl;
	return -1;
	}
	*/

	// glad populates global constants after loading to indicate,
	// if a certain extension/version is available.
	LOG(logClient, platform::Info) << "OpenGL version: " << GLVersion.major << "." << GLVersion.minor;

	/*if (GLAD_GL_EXT_framebuffer_multisample) {
	}*/

	glViewport(0,0, width, height);
	glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}


InputGLFW::InputGLFW(WindowGLFW* window) : window(window), 
anyPressedKey(KEY_UNKNOWN), anyPressedButton(InvalidButton), logClient(getLogServer())
{
	logClient.setPrefix("InputGLFW");
}

InputGLFW::InputGLFW(const InputGLFW& other) : window(other.window), logClient(other.logClient),
	anyPressedKey(other.anyPressedKey), anyPressedButton(other.anyPressedButton)
{
}

InputGLFW::~InputGLFW()
{
}

Input::Button InputGLFW::getAnyPressedButton()
{
	return anyPressedButton;
}

Input::Key InputGLFW::getAnyPressedKey()
{
	return anyPressedKey;
}

bool InputGLFW::isDown(Button button)
{
	int glfwButton = WindowSystemGLFW::toGLFWbutton(button);
	if (glfwButton == GLFW_KEY_UNKNOWN) return false;
	return glfwGetMouseButton(window->getSource(), glfwButton) != GLFW_RELEASE;
}

bool InputGLFW::isDown(Key key)
{
	int glfwKey = WindowSystemGLFW::toGLFWkey(key);
	if (glfwKey == GLFW_KEY_UNKNOWN) return false;
	return glfwGetKey(window->getSource(), glfwKey) != GLFW_RELEASE;
}

bool InputGLFW::isPressed(Button button)
{
	int glfwButton = WindowSystemGLFW::toGLFWbutton(button);
	if (glfwButton == GLFW_KEY_UNKNOWN) return false;
	return window->isButtonPressed(button);
}

bool InputGLFW::isPressed(Key key)
{
	int glfwKey = WindowSystemGLFW::toGLFWkey(key);
	if (glfwKey == GLFW_KEY_UNKNOWN) return false;
	return window->isKeyPressed(glfwKey);
}

bool InputGLFW::isReleased(Button button)
{
	int glfwButton = WindowSystemGLFW::toGLFWbutton(button);
	if (glfwButton == GLFW_KEY_UNKNOWN) return false;
	return window->isButtonReleased(button);
}

bool InputGLFW::isReleased(Key key)
{
	int glfwKey = WindowSystemGLFW::toGLFWkey(key);
	if (glfwKey == GLFW_KEY_UNKNOWN) return false;
	return window->isKeyReleased(glfwKey);
}

void InputGLFW::scrollCallback(double xOffset, double yOffset)
{
	frameScrollOffsetX = xOffset;
	frameScrollOffsetY = yOffset;

	if (frameScrollOffsetX || frameScrollOffsetY)
	{
		informScrollListeners(xOffset, yOffset);
	}
}

void InputGLFW::frameUpdate()
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

void InputGLFW::resetForFrame()
{
	// clear state of keys  and mouse buttons before polling, as callbacks are called during polling!
	// TODO move releasedKeys/pressedKeys to this class
	window->releasedKeys.clear();
	window->pressedKeys.clear();
	window->releasedButtons.clear();
	window->pressedButtons.clear();

	frameScrollOffsetX = 0;
	frameScrollOffsetY = 0;
}

void InputGLFW::setMousePosition(int xPos, int yPos)
{
	glfwSetCursorPos(window->getSource(), static_cast<double>(xPos), static_cast<double>(yPos));
	mouseXabsolut = xPos;
	mouseYabsolut = yPos;
	frameMouseXOffset = 0;
	frameMouseYOffset = 0;
}

void InputGLFW::setWindow(WindowGLFW* window)
{
	this->window = window;
}