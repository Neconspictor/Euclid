#include <platform/Window.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>

Window::Window(WindowStruct const& description): renderer(nullptr),
logClient(platform::getLogServer())
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
	vSync = description.vSync;

	logClient.setPrefix("[Window]");
}

void Window::setTitle(const std::string& newTitle)
{
	title = newTitle;
}

const std::string& Window::getTitle() const
{
	return title;
}

Renderer::Viewport Window::getViewport() const
{
	if (!renderer.get()) return {0,0,0,0};
	return renderer->getViewport();
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