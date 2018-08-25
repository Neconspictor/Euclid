#include <nex/Input.hpp>
#include <nex/logging/GlobalLoggingServer.hpp>

using namespace nex;

Input::Input() :
	logClient(getLogServer())
{
	frameMouseXOffset = frameMouseYOffset = 0;
	frameScrollOffsetX = 0;
	frameScrollOffsetY = 0;
	mouseXabsolut = 0;
	mouseYabsolut = 0;
	firstMouseInput = true;
	m_windowHasFocus = true;

	logClient.setPrefix("[Input]");
}

Input::~Input()
{
}

Input::WindowRefreshConnection Input::addRefreshCallback(const WindowRefreshCallback & callback)
{
	return windowRefreshContainer.addCallback(callback);
}

CallbackContainer<void(Window*)>::Callback Input::addWindowCloseCallback(const WindowClose::CallbackFunc& callback)
{
	return m_windowCloseCallbacks.addCallback(callback);
}

Input::ResizeConnection Input::addResizeCallback(const ResizeCallback& callback)
{
	return windowResizeContainer.addCallback(callback);
}

Input::ScrollConnection Input::addScrollCallback(const ScrollCallback& callback)
{
	return scrollContainer.addCallback(callback);
}

Input::WindowFocusConnection Input::addWindowFocusCallback(const WindowFocusCallback& callback)
{
	return windowFocusChanged.addCallback(callback);
}


MouseOffset Input::getFrameMouseOffset()
{
	MouseOffset offset;
	offset.xAbsolute = mouseXabsolut;
	offset.yAbsolute = mouseYabsolut;
	offset.xOffset = frameMouseXOffset;
	offset.yOffset = frameMouseYOffset;
	return offset;
}


double Input::getFrameScrollOffsetX()
{
	return frameScrollOffsetX;
}

double Input::getFrameScrollOffsetY()
{
	return frameScrollOffsetY;
}

void Input::informScrollListeners(double scrollX, double scrollY)
{
	for (const ScrollConnection& connection : scrollContainer.getCallbacks())
	{
		ScrollCallback callback = connection.get()->getCallbackFunc();
		callback(scrollX, scrollY);
	}
}

void Input::informRefreshListeners()
{
	for (WindowRefreshConnection connection : windowRefreshContainer.getCallbacks())
	{
		WindowRefreshCallback callback = connection.get()->getCallbackFunc();
		callback();
	}
}

void Input::informResizeListeners(int width, int height)
{
	for (ResizeConnection connection : windowResizeContainer.getCallbacks())
	{
		ResizeCallback callback = connection.get()->getCallbackFunc();
		callback(width, height);
	}
}

void Input::informWindowFocusListeners(bool receivedFocus)
{
	for (WindowFocusConnection sharedItem : windowFocusChanged.getCallbacks())
	{
		WindowFocusCallback callback = sharedItem.get()->getCallbackFunc();
		callback(getWindow(), receivedFocus);
	}
}

void Input::informWindowCloseListeners()
{
	for (WindowClose::Callback callback : m_windowCloseCallbacks.getCallbacks())
	{
		callback->getCallbackFunc()(getWindow());
	}
}


void Input::removeScrollConnection(const ScrollConnection& connection)
{
	scrollContainer.removeCallback(connection);
}

void Input::removeRefreshCallback(const WindowRefreshConnection & connection)
{
	windowRefreshContainer.removeCallback(connection);
}

void Input::removeWindowCloseCallback(const WindowClose::Callback& callback)
{
	m_windowCloseCallbacks.removeCallback(callback);
}

void Input::removeResizeCallback(const ResizeConnection& connection)
{
	windowResizeContainer.removeCallback(connection);
}

void Input::removeWindowFocusCallback(const WindowFocusConnection& connection)
{
	windowFocusChanged.removeCallback(connection);
}

void Input::resetMouseMovement()
{
	frameMouseXOffset = 0;
	frameMouseYOffset = 0;
}

void Input::setMousePosition(int xPos, int yPos)
{
	frameMouseXOffset += xPos - mouseXabsolut;
	frameMouseYOffset += yPos - mouseYabsolut;
	mouseXabsolut = xPos;
	mouseYabsolut = yPos;
}

bool Input::windowHasFocus()
{
	return m_windowHasFocus;
}