// glad has to be included before glfw
#include <nex/opengl/opengl.hpp>
#include <GLFW/glfw3.h>
#include <nex/opengl/window_system/glfw/InputGLFW.hpp>
#include <nex/opengl/window_system/glfw/WindowGLFW.hpp>
#include <nex/opengl/window_system/glfw/SubSystemProviderGLFW.hpp>
#include <nex/util/ExceptionHandling.hpp>

#ifdef WIN32
#define GLFW_EXPOSE_NATIVE_WIN32 1
#include <GLFW/glfw3native.h>
#endif


using namespace std;
using namespace nex;


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

void WindowGLFW::activate()
{
	glfwMakeContextCurrent(window);
}

void WindowGLFW::close()
{
	if (!isOpen()) return;
	
	glfwSetWindowShouldClose(window, GLFW_TRUE);
	inputDevice.informWindowCloseListeners();

	if (!isOpen())
	{
		inputDevice.disableCallbacks();
		//inputDevice.removeCallbacks();
	}

}

void* WindowGLFW::getNativeWindow()
{
#ifdef WIN32
	return glfwGetWin32Window(window);
#endif

	// just return default value
	return Window::getNativeWindow();
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
	if (!SubSystemProviderGLFW::get()->init())
	{
		throw_with_trace(runtime_error("WindowGLFW: Error: Couldn't initialize GLFWSystem!"));
	}

	createOpenGLWindow();
}

bool WindowGLFW::isOpen()
{
	mIsClosed = static_cast<bool>(glfwWindowShouldClose(window));
	return !mIsClosed;
}

void WindowGLFW::minimize()
{
	glfwIconifyWindow(window);
}

void WindowGLFW::release()
{
	inputDevice.removeCallbacks();
	glfwDestroyWindow(window);
	window = nullptr;
}

void WindowGLFW::reopen()
{
	if (window != nullptr && !isOpen())
	{
		glfwSetWindowShouldClose(window, GLFW_FALSE);
		inputDevice.enableCallbacks();
	}
}

void WindowGLFW::resize(unsigned newWidth, unsigned newHeight)
{
	//mConfig.frameBufferWidth = newWidth;
	//mConfig.frameBufferHeight = newHeight;
	glfwSetWindowSize(window, static_cast<int>(newWidth), static_cast<int>(newHeight));

	//TODO
	//inputDevice.informVirtualDimensionResizeListeners(mConfig.virtualScreenWidth, mConfig.virtualScreenHeight);
}

void WindowGLFW::setCursorPosition(int xPos, int yPos)
{
	inputDevice.setMousePosition(xPos, yPos);
}

void WindowGLFW::setFocus(bool focus)
{
	if (focus) {
		LOG(mLogger, nex::Debug) << "gained focus!";
	} else {
		LOG(mLogger, nex::Debug) << "lost focus!";
	}

	m_hasFocus = focus;
	inputDevice.informWindowFocusListeners(focus);
}

void WindowGLFW::setFullscreen()
{
	glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), mConfig.posX, mConfig.posY, mConfig.frameBufferWidth, mConfig.frameBufferHeight, mConfig.refreshRate);
	mConfig.fullscreen = true;
}

void WindowGLFW::setFrameBufferSize(unsigned width, unsigned height)
{
	mConfig.frameBufferWidth = width;
	mConfig.frameBufferHeight = height;
	inputDevice.inforrmFrameBufferResiteListeners(width, height);
}

void WindowGLFW::setVirtualScreenDimension(unsigned width, unsigned height)
{
	mConfig.virtualScreenWidth = width;
	mConfig.virtualScreenHeight = height;
	inputDevice.informVirtualDimensionResizeListeners(width, height);
}

void WindowGLFW::setTitle(const string& newTitle)
{
	Window::setTitle(newTitle);
	glfwSetWindowTitle(window, mConfig.title.c_str());
}

void WindowGLFW::setVisible(bool visible)
{
	if (mConfig.visible == visible) return;

	mConfig.visible = visible;

	if (mConfig.visible)
		glfwShowWindow(window);
	else
		glfwHideWindow(window);
}

void WindowGLFW::setVsync(bool vsync)
{
	Window::setVsync(vsync);

	if (!mConfig.vSync)
	{
		glfwSwapInterval(0);
	}
	else
	{
		glfwSwapInterval(1);
	}
}

void WindowGLFW::setWindowed()
{	
	glfwSetWindowMonitor(window, nullptr, mConfig.posX, mConfig.posY, mConfig.frameBufferWidth, mConfig.frameBufferHeight, mConfig.refreshRate);
	
	int top, left, bottom, right;
	glfwGetWindowFrameSize(window, &left, &top, &right, &bottom);
	glfwSetWindowPos(window, mConfig.posX + left, mConfig.posY + top);

	mConfig.fullscreen = false;

	refreshWindowWithoutCallbacks();

	//now inform listeners
	//setSize(width, height);
	//informResizeListeners(width, height);
}

void WindowGLFW::showCursor(bool show)
{
	if (show) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	} else {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
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
	//SubSystemProviderGLFW* system = SubSystemProviderGLFW::get();
	//system->removeSizeCallback(this);

	//register a dummy callback that does nothing
	//system->registerRefreshCallback(this, [](GLFWwindow*){}); 

	// change the size of the window temporarily in order to trigger the operating system to repaint the window
	// To avoid issues we don't propagate this temporary change
	const auto widthBackup = mConfig.frameBufferWidth;

	glfwSetWindowSize(window, 0, mConfig.frameBufferHeight);

	int top, left, bottom, right;
	glfwGetWindowFrameSize(window, &left, &top, &right, &bottom);

	int newWidth = right - left;

	// nothing has changed
	if (newWidth == widthBackup) {
		glfwSetWindowSize(window, newWidth+1, mConfig.frameBufferHeight);
	}

	// hack!!! so that opengl recognises that it should update 
	// TODO: Do not use opengl commands in the window class!
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glViewport(0, 0, mConfig.frameBufferWidth, mConfig.frameBufferHeight);
	glScissor(0, 0, mConfig.frameBufferWidth, mConfig.frameBufferHeight);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	glfwSwapBuffers(window);

	//now restore the original window size
	glfwSetWindowSize(window, widthBackup, mConfig.frameBufferHeight);

	// restore the callbacks
	inputDevice.enableCallbacks();
}

void WindowGLFW::createOpenGLWindow()
{
	//TODO
	/*glfwWindowHint(GLFW_VISIBLE, m_isVisible ? GLFW_TRUE : GLFW_FALSE);*/

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	

	/*glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);*/
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	auto* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);


	//glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	//glfwWindowHint(GLFW_SAMPLES, GLFW_DONT_CARE);
	glfwWindowHint(GLFW_REFRESH_RATE, mConfig.refreshRate);



#ifdef EUCLID_ALL_OPTIMIZATIONS
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_FALSE);
	glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_TRUE);
#else
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_FALSE);
#endif



	//glfwWindowHint(GLFW_DECORATED, GL_FALSE);
	//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


	window = glfwCreateWindow(mConfig.virtualScreenWidth, mConfig.virtualScreenHeight, mConfig.title.c_str(), nullptr, nullptr);

	if (!window)
	{
		throw_with_trace(runtime_error("WindowGLFW: Error: Couldn't create GLFWwindow!"));
	}


	// update virtual screen and framebuffer dimensions
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	mConfig.frameBufferWidth = static_cast<unsigned>(w);
	mConfig.frameBufferHeight = static_cast<unsigned>(h);

	glfwGetWindowSize(window, &w, &h);
	mConfig.virtualScreenWidth = static_cast<unsigned>(w);
	mConfig.virtualScreenHeight = static_cast<unsigned>(h);

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
		throw_with_trace(runtime_error("WindowGLFW::createOpenGLWindow(): Failed to initialize OpenGL context"));
	}

	setVsync(mConfig.vSync);

	// Alternative use the builtin loader, e.g. if no other loader function is available
	/*
	if (!gladLoadGL()) {
	std::cout << "Failed to initialize OpenGL context" << std::endl;
	return -1;
	}
	*/

	// glad populates global constants after loading to indicate,
	// if a certain extension/version is available.
	LOG(mLogger, nex::Info) << "OpenGL version: " << GLVersion.major << "." << GLVersion.minor;

	/*if (GLAD_GL_EXT_framebuffer_multisample) {
	}*/

	glViewport(0, 0, mConfig.frameBufferWidth, mConfig.frameBufferHeight);
	glScissor(0, 0, mConfig.frameBufferWidth, mConfig.frameBufferHeight);
	glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}