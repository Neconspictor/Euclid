#include <platform/Input.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>

using namespace platform;

Input* Input::instance = nullptr;

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

void Input::informScrollListeners(double scrollX, double scrollY)
{
	for (const ScrollConnection& connection : scrollContainer.getCallbacks())
	{
		ScrollCallback callback = connection.get()->getCallback();
		callback(scrollX, scrollY);
	}
}

Input::~Input()
{
}

Input::ScrollConnection Input::addScrollCallback(const ScrollCallback& callback)
{
	return scrollContainer.addCallback(callback);
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

void Input::removeScrollConnection(const ScrollConnection& connection)
{
	scrollContainer.removeCallback(connection);
}

void Input::setMousePosition(int xPos, int yPos)
{
	mouseXabsolut = xPos;
	mouseYabsolut = yPos;
}

bool Input::windowHasFocus()
{
	return m_windowHasFocus;
}


double Input::getFrameScrollOffsetX()
{
	return frameScrollOffsetX;
}


double Input::getFrameScrollOffsetY()
{
	return frameScrollOffsetY;
}