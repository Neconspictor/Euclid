// glad has to be included before glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <platform/window_system/glfw/InputGLFW.hpp>
#include <platform/window_system/glfw/WindowGLFW.hpp>
#include <platform/window_system/glfw/WindowSystemGLFW.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <functional>


using namespace std;
using namespace platform;


WindowGLFW::WindowGLFW(WindowStruct const& desc) :
	Window(desc), window(nullptr), inputDevice(this), m_hasFocus(true)
{
}

WindowGLFW::WindowGLFW(WindowGLFW && o) : Window(move(o)), inputDevice(move(o.inputDevice))
{
	window = o.window;
	inputDevice.setWindow(this);
	m_hasFocus = o.m_hasFocus;

	//o.window = nullptr;
}

WindowGLFW & WindowGLFW::operator=(WindowGLFW && o)
{
	if (this == &o) return *this;
	inputDevice = move(o.inputDevice);
	window = o.window;
	m_hasFocus = o.m_hasFocus;

	o.window = nullptr;

	return *this;
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
	inputDevice.disableCallbacks();
	inputDevice.removeCallbacks();
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

void WindowGLFW::release()
{
	glfwDestroyWindow(window);
	window = nullptr;
}

void WindowGLFW::resize(int newWidth, int newHeight)
{
	width = newWidth;
	height = newHeight;
	glfwSetWindowSize(window, width, height);
	inputDevice.informResizeListeners(width, height);
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
	inputDevice.informWindowFocusListeners(focus);
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
	inputDevice.informResizeListeners(width, height);
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


void WindowGLFW::refreshWindowWithoutCallbacks()
{

	// assure that o callbacks get called!
	inputDevice.disableCallbacks();
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
	inputDevice.enableCallbacks();
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