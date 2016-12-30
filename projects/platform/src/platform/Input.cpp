#include <platform/Input.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <platform/Platform.hpp>

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
	for (ScrollConnection& connection : scrollContainer.getCallbacks())
	{
		ScrollCallback callback = connection.get()->getCallback();
		callback(scrollX, scrollY);
	}
}

Input::Input(const Input& other) : Input()
{

}

Input::~Input()
{
}

Input::ScrollConnection Input::addScrollCallback(const ScrollCallback& callback)
{
	return scrollContainer.addCallback(callback);
}

void Input::onWindowsFocus(Window* window, int focused)
{
	instance->m_windowHasFocus = !!focused;

	if (instance->m_windowHasFocus)
	{
		//glfwSetCursorPos(window, 400, 300);
		//TODO set window cursor position!
	}
}

void Input::onScroll(Window* window, double xoffset, double yoffset)
{
	/*for (ScrollListener* listener : instance->scrollListeners)
	{
		listener->onScroll(yoffset);
	}*/
}

/*void Input::updateOnFrame(GLFWwindow* window, double frameTime)
{
	double xPos, yPos;
	glfwGetCursorPos(window, &xPos, &yPos);
	instance->frameMouseXOffset = xPos - 400;
	instance->frameMouseYOffset = yPos - 300;
	glfwSetCursorPos(window, 400, 300);
}*/

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

/*void Input::addScrollListener(ScrollListener& listener)
{
	bool contains = find(scrollListeners.begin(), scrollListeners.end(), &listener) != scrollListeners.end();
	if (!contains)
	{
		scrollListeners.push_back(&listener);
	}
}

void Input::removeScrollListener(ScrollListener& listener)
{
	std::list<ScrollListener*>::iterator contains = find(scrollListeners.begin(), scrollListeners.end(), &listener);

	if (contains != scrollListeners.end())
	{
		scrollListeners.erase(contains);
	}
}*/