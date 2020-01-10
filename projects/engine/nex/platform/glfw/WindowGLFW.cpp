
#include <nex/platform/glfw/InputGLFW.hpp>
#include <nex/platform/glfw/WindowGLFW.hpp>
#include <nex/platform/glfw/SubSystemProviderGLFW.hpp>
#include <nex/util/ExceptionHandling.hpp>

#ifdef WIN32
#define GLFW_EXPOSE_NATIVE_WIN32 1
#include <GLFW/glfw3native.h>
#endif


using namespace std;
using namespace nex;


Cursor::Cursor(StandardCursorType type) : mPimpl(std::make_unique<Impl>(translate(type))){}

Cursor::~Cursor() = default;

Cursor::Impl* Cursor::getImpl()
{
	return mPimpl.get();
}

StandardCursorTypeGLFW nex::translate(StandardCursorType type)
{
	static StandardCursorTypeGLFW table[]
	{
		StandardCursorTypeGLFW::Arrow,
		StandardCursorTypeGLFW::Hand,
		StandardCursorTypeGLFW::TextIBeam,
		StandardCursorTypeGLFW::CrossHair,
		StandardCursorTypeGLFW::HorizontalResize,
		StandardCursorTypeGLFW::VerticalResize,
	};

	static const auto tableSize = sizeof(table) / sizeof(StandardCursorTypeGLFW);
	static const auto targetSize = static_cast<unsigned>(StandardCursorType::LAST) - static_cast<unsigned>(StandardCursorType::FIRST) + 1;
	static_assert(tableSize == targetSize, "StandardCursorType and StandardCursorTypeGLFW don't match!");

	return table[static_cast<unsigned>(type)];
}

CursorStateGLFW nex::translate(CursorState state)
{
	static CursorStateGLFW table[]
	{
		CursorStateGLFW::Disabled,
		CursorStateGLFW::Hidden,
		CursorStateGLFW::Normal,
	};

	static const auto tableSize = sizeof(table) / sizeof(CursorStateGLFW);
	static const auto targetSize = static_cast<unsigned>(CursorState::LAST) - static_cast<unsigned>(CursorState::FIRST) + 1;
	static_assert(tableSize == targetSize, "CursorState and CursorStateGLFW don't match!");

	return table[static_cast<unsigned>(state)];
}

CursorState nex::translate(CursorStateGLFW state)
{
	static const std::map<CursorStateGLFW, CursorState> table = {
		{CursorStateGLFW::Disabled, CursorState::Disabled},
		{CursorStateGLFW::Hidden, CursorState::Hidden},
		{CursorStateGLFW::Normal, CursorState::Normal}
	};

	static const auto tableSize = 3;
	static const auto targetSize = static_cast<unsigned>(CursorState::LAST) - static_cast<unsigned>(CursorState::FIRST) + 1;
	static_assert(tableSize == targetSize, "CursorStateGLFW and CursorState don't match!");

	return table.at(state);
}

Cursor::Impl::Impl(StandardCursorTypeGLFW shape) : mCursor(nullptr)
{
	mCursor = glfwCreateStandardCursor(static_cast<int>(shape));
	assert(mCursor != nullptr);
}

Cursor::Impl::~Impl()
{
	if (mCursor != nullptr)
		glfwDestroyCursor(mCursor);
	mCursor = nullptr;
}

GLFWcursor* Cursor::Impl::getCursor()
{
	return mCursor;
}

WindowGLFW::WindowGLFW(WindowStruct const& desc) :
	Window(desc), window(nullptr), inputDevice(this, desc.language), m_hasFocus(true), mCursor(nullptr)
{
}

WindowGLFW::WindowGLFW(WindowGLFW && o) : Window(move(o)), inputDevice(move(o.inputDevice)),mCursor(std::exchange(o.mCursor, nullptr))
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
	mCursor = std::exchange(o.mCursor, nullptr);

	return *this;
}

void WindowGLFW::activate(bool deactivate)
{
	if (!deactivate)
		glfwMakeContextCurrent(window);
	else
		glfwMakeContextCurrent(nullptr);
}

void WindowGLFW::close(bool silent)
{
	if (!isOpen()) return;
	
	glfwSetWindowShouldClose(window, GLFW_TRUE);
	if (!silent) inputDevice.informWindowCloseListeners();

	if (!isOpen())
	{
		inputDevice.disableCallbacks();
		//inputDevice.removeCallbacks();
	}

}

const Cursor* WindowGLFW::getCursor() const
{
	return mCursor;
}

CursorState WindowGLFW::getCursorState() const
{
	auto stateGLFW = glfwGetInputMode(window, GLFW_CURSOR);
	return translate(static_cast<CursorStateGLFW>(stateGLFW));
}

void WindowGLFW::setCursor(Cursor* cursor)
{
	assert(cursor != nullptr);
	glfwSetCursor(window, cursor->getImpl()->getCursor());
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

bool WindowGLFW::hasFocus() const
{
	return m_hasFocus;
}

void WindowGLFW::init()
{
	if (!SubSystemProviderGLFW::get()->init())
	{
		throw_with_trace(runtime_error("WindowGLFW: Error: Couldn't initialize GLFWSystem!"));
	}

	createWindowWithRenderContext();

	// update virtual screen and framebuffer dimensions
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	mConfig.frameBufferWidth = static_cast<unsigned>(w);
	mConfig.frameBufferHeight = static_cast<unsigned>(h);

	glfwGetWindowSize(window, &w, &h);
	mConfig.virtualScreenWidth = static_cast<unsigned>(w);
	mConfig.virtualScreenHeight = static_cast<unsigned>(h);

	//activate();
	//setVsync(mConfig.vSync);
	//activate(true);
}

bool WindowGLFW::isOpen() const
{
	return !static_cast<bool>(glfwWindowShouldClose(window));
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
	glfwSetWindowSize(window, static_cast<int>(newWidth), static_cast<int>(newHeight));
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
	inputDevice.informFrameBufferResizeListeners(width, height);
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
}

void WindowGLFW::showCursor(CursorState state)
{
	glfwSetInputMode(window, GLFW_CURSOR, static_cast<int>(translate(state)));
}

void WindowGLFW::swapBuffers()
{
	//glViewport(0, 0, 800, 600);
	//glBindSampler(0, 0);
	glfwSwapBuffers(window);
}


void WindowGLFW::refreshWindowWithoutCallbacks()
{
	// assure that no callbacks get called!
	inputDevice.disableCallbacks();

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

	//now restore the original window size
	glfwSetWindowSize(window, widthBackup, mConfig.frameBufferHeight);

	// restore the callbacks
	inputDevice.enableCallbacks();
}