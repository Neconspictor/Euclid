// glad has to be included before glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <platform/window_system/glfw/WindowGLFW.hpp>
#include <platform/window_system/glfw/WindowSystemGLFW.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <functional>


using namespace std;
using namespace platform;


WindowGLFW::WindowGLFW(WindowStruct const& desc, Renderer& renderer) :
	Window(desc, renderer), window(nullptr), inputDevice(this), m_hasFocus(true)
{
}

WindowGLFW::~WindowGLFW()
{
	cout << "called WindowGLFW::~WindowGLFW()" << endl;
}

void WindowGLFW::activate()
{
	//TODO
	if(renderer->getType() == OPENGL)
	{
		glfwMakeContextCurrent(window);
	}
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

	if (renderer->getType() == OPENGL)
	{
		createOpenGLWindow();
	}
	else
	{
		createNoAPIWindow();
	}
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

void WindowGLFW::onCharMods(unsigned codepoint, int mods)
{
	for (auto& c : charModsCallbacks)
	{
		c(this->window, codepoint, mods);
	}
}

void WindowGLFW::onKey(int key, int scancode, int action, int mods)
{
	for (auto& c : keyCallbacks)
	{
		c(this->window, key, scancode, action, mods);
	}
}

void WindowGLFW::onMouse(int button, int state, int mods)
{
	for (auto& c : mouseCallbacks)
	{
		c(this->window, button, state, mods);
	}
}

void WindowGLFW::registerCallbacks()
{
	using namespace placeholders;

	inputDevice.setWindow(this);

	auto charModsCallback = bind(&InputGLFW::charModsCallback, inputDevice, _1, _2, _3);
	auto focusCallback = bind(&InputGLFW::focusCallback, inputDevice, _1, _2);
	auto keyCallback = bind(&InputGLFW::keyCallback, inputDevice, _1, _2, _3, _4, _5);
	auto mouseCallback = bind(&InputGLFW::mouseCallback, inputDevice, _1, _2, _3, _4);
	auto scrollCallback = bind(&InputGLFW::scrollCallback, inputDevice, _1, _2, _3);
	auto sizeCallback = bind(&InputGLFW::sizeCallback, inputDevice, _1, _2, _3);

	WindowSystemGLFW::get()->registerCharModsCallback(this, charModsCallback);
	WindowSystemGLFW::get()->registerFocusCallback(this, focusCallback);
	WindowSystemGLFW::get()->registerKeyCallback(this, keyCallback);
	WindowSystemGLFW::get()->registerMouseCallback(this, mouseCallback);
	WindowSystemGLFW::get()->registerScrollCallback(this, scrollCallback);
	WindowSystemGLFW::get()->registerSizeCallback(this, sizeCallback);
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

void WindowGLFW::registerMouseCallback(std::function<MouseCallback> callback)
{
	mouseCallbacks.push_back(callback);
}

void WindowGLFW::removeCallbacks()
{
	using namespace placeholders;

	WindowSystemGLFW* system = WindowSystemGLFW::get();

	system->removeCharModsCallback(this);
	system->removeFocusCallbackCallback(this);
	system->removeKeyCallback(this);
	system->removeMouseCallback(this);
	system->removeScrollCallback(this);
	system->removeSizeCallback(this);
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
	m_hasFocus = focus;
	informWindowFocusListeners(focus);
}

void WindowGLFW::setFullscreen()
{
	removeCallbacks();

	glfwWindowHint(GLFW_VISIBLE, m_isVisible ? GLFW_TRUE : GLFW_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	//glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);

	//glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_REFRESH_RATE, this->refreshRate);

	GLFWwindow* newWindow = glfwCreateWindow(width, height, title.c_str(), glfwGetPrimaryMonitor(), window);

	glfwDestroyWindow(window);
	window = newWindow;

	if (!vSync)
	{
		glfwSwapInterval(0);
	}
	else
	{
		glfwSwapInterval(1);
	}

	registerCallbacks();
	glfwMakeContextCurrent(window);
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
	removeCallbacks();

	glfwWindowHint(GLFW_VISIBLE, m_isVisible ? GLFW_TRUE : GLFW_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);


	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);

	//glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_REFRESH_RATE, this->refreshRate);

	GLFWwindow* newWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, window);
	
	glfwDestroyWindow(window);
	window = newWindow;

	if (!vSync)
	{
		glfwSwapInterval(0);
	}
	else
	{
		glfwSwapInterval(1);
	}

	registerCallbacks();

	glfwMakeContextCurrent(window);

	int top, left, bottom, right;
	glfwGetWindowFrameSize(window, &left, &top, &right, &bottom);

	glfwSetWindowPos(window, this->posX + left, this->posY + top);

	fullscreen = false;
}

void WindowGLFW::swapBuffers()
{
	//glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glViewport(0, 0, 800, 600);
	glBindSampler(0, 0);
	glfwSwapBuffers(window);
}

void WindowGLFW::createNoAPIWindow()
{
	// create no opengl context -> caller is responsible!
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(this->width, this->height, this->title.c_str(), nullptr, nullptr);

	if (!window)
	{
		WindowSystemGLFW::get()->terminate();
		throw runtime_error("WindowGLFW: Error: Couldn't create GLFWwindow!");
	}
}

void WindowGLFW::createOpenGLWindow()
{
	//TODO
	/*glfwWindowHint(GLFW_VISIBLE, m_isVisible ? GLFW_TRUE : GLFW_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_REFRESH_RATE, 59);

	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);


	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);*/

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	//glfwWindowHint(GLFW_SAMPLES, 4);
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

#if defined(NANOGUI_GLAD)
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		throw std::runtime_error("Could not initialize GLAD!");
	glGetError(); // pull and ignore unhandled errors like GL_INVALID_ENUM
#endif

	// Load all OpenGL functions using the glfw loader function
	// If you use SDL you can use: https://wiki.libsdl.org/SDL_GL_GetProcAddress
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
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

	glClearColor(0.2f, 0.25f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}


InputGLFW::InputGLFW(WindowGLFW* window) : window(window), logClient(getLogServer())
{
	logClient.setPrefix("InputGLFW");
}

InputGLFW::InputGLFW(const InputGLFW& other) : window(other.window), logClient(other.logClient)
{
}

Input::Button InputGLFW::getAnyPressedButton()
{
	//TODO
	return InvalidButton;
}

Input::Key InputGLFW::getAnyPressedKey()
{
	//TODO
	return KEY_UNKNOWN;
}

bool InputGLFW::isDown(Button button)
{
	WindowSystemGLFW* system = WindowSystemGLFW::get();
	int glfwButton = system->toGLFWbutton(button);
	if (glfwButton == GLFW_KEY_UNKNOWN) return false;
	//return system->isButtonDown(button);
	return glfwGetMouseButton(window->getSource(), glfwButton) != GLFW_RELEASE;
}

bool InputGLFW::isDown(Key key)
{
	WindowSystemGLFW* system = WindowSystemGLFW::get();
	int glfwKey = system->toGLFWkey(key);
	if (glfwKey == GLFW_KEY_UNKNOWN) return false;
	return glfwGetKey(window->getSource(), glfwKey) != GLFW_RELEASE;
}

bool InputGLFW::isPressed(Button button)
{
	WindowSystemGLFW* system = WindowSystemGLFW::get();
	int glfwButton = system->toGLFWbutton(button);
	if (glfwButton == GLFW_KEY_UNKNOWN) return false;
	return system->isButtonPressed(button);
}

bool InputGLFW::isPressed(Key key)
{
	WindowSystemGLFW* system = WindowSystemGLFW::get();
	int glfwKey = system->toGLFWkey(key);
	if (glfwKey == GLFW_KEY_UNKNOWN) return false;
	return system->isKeyPressed(glfwKey);
}

bool InputGLFW::isReleased(Button button)
{
	WindowSystemGLFW* system = WindowSystemGLFW::get();
	int glfwButton = system->toGLFWbutton(button);
	if (glfwButton == GLFW_KEY_UNKNOWN) return false;
	return system->isButtonReleased(button);
}

bool InputGLFW::isReleased(Key key)
{
	WindowSystemGLFW* system = WindowSystemGLFW::get();
	int glfwKey = system->toGLFWkey(key);
	if (glfwKey == GLFW_KEY_UNKNOWN) return false;
	return system->isKeyReleased(glfwKey);
}

void InputGLFW::charModsCallback(GLFWwindow* window, unsigned codepoint, int mods)
{
	this->window->onCharMods(codepoint, mods);
}

void InputGLFW::focusCallback(GLFWwindow* window, int hasFocus)
{
	bool focus = hasFocus == GLFW_TRUE ? true : false;
	if (focus)
	{
		LOG(logClient, platform::Debug) << "gained focus!";
	} else
	{
		LOG(logClient, platform::Debug) << "lost focus!";
	}

	this->window->setFocus(focus);
}

void InputGLFW::mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	this->window->onMouse(button, action, mods);
}

void InputGLFW::scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	frameScrollOffsetX = xOffset;
	frameScrollOffsetY = yOffset;

	if (frameScrollOffsetX || frameScrollOffsetY)
	{
		informScrollListeners(xOffset, yOffset);
	}


}

void InputGLFW::sizeCallback(GLFWwindow* window, int width, int height)
{
	this->window->setSize(width, height);
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
}

void InputGLFW::resetForFrame()
{
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

void InputGLFW::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	this->window->onKey(key, scancode, action, mods);
}

void InputGLFW::setWindow(WindowGLFW* window)
{
	this->window = window;
}