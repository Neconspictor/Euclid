#include "platform//Window.hpp"

Window::Window(WindowStruct const& description): renderer(nullptr)
{
	width = description.width;
	height = description.height;
	colorBitDepth = description.colorBitDepth;
	fullscreen = description.fullscreen;
	refreshRate = description.refreshRate;
	posX = description.posX;
	posY = description.posY;
	title = description.title;
	m_isVisible = description.visible;
	m_isOpen = true;
	m_hasFocus = false;
}

Window::WindowFocusConnection Window::addWindowFocusCallback(const WindowFocusCallback& callback)
{
	return windowFocusChanged.addCallback(callback);
}

void Window::removeWindowFocusCallback(const WindowFocusConnection& connection)
{
	windowFocusChanged.removeCallback(connection);
}


void Window::informWindowFocusListeners(bool receivedFocus)
{
	for (WindowFocusChanged::SharedItem sharedItem : windowFocusChanged.getCallbacks())
	{
		WindowFocusChanged::Callback callback = sharedItem.get()->getCallback();
		callback(this, receivedFocus);
	}
}