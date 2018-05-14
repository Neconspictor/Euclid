#include <platform/Window.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>

Window::Window(WindowStruct const& description):
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

bool Window::isInFullscreenMode()
{
	return fullscreen;
}