#include <platform/Window.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>

Window::Window(WindowStruct const& description, Renderer& renderer): renderer(&renderer),
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

int Window::getHeight() const
{
	return height;
}

int Window::getPosX() const
{
	return posX;
}

int Window::getPosY() const
{
	return posY;
}

int Window::getWidth() const
{
	return width;
}

const std::string& Window::getTitle() const
{
	return title;
}

Renderer::Viewport Window::getViewport() const
{
	return renderer->getViewport();
}

bool Window::isInFullscreenMode()
{
	return fullscreen;
}

Window::ResizeConnection Window::addResizeCallback(const ResizeCallback& callback)
{
	return windowResizeContainer.addCallback(callback);
}

Window::WindowFocusConnection Window::addWindowFocusCallback(const WindowFocusCallback& callback)
{
	return windowFocusChanged.addCallback(callback);
}

void Window::removeResizeCallback(const ResizeConnection& connection)
{
	windowResizeContainer.removeCallback(connection);
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

void Window::informResizeListeners(int width, int height)
{
	for (ResizeConnection connection : windowResizeContainer.getCallbacks())
	{
		ResizeCallback callback = connection.get()->getCallback();
		callback(width, height);
	}
}