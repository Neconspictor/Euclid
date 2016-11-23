#include <platform/Input.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>

using namespace platform;

Input* Input::instance = nullptr;

Input::Input() :
	logClient(getLogServer())
{
	frameMouseXOffset = frameMouseYOffset = 0;
	frameScrollOffset = 0;
	mouseXabsolut = 0;
	mouseYabsolut = 0;
	firstMouseInput = true;
	m_windowHasFocus = true;

	logClient.setPrefix("[Input]");
}

void Input::informScrollListeners(float scrollDiff)
{
	for (ScrollConnection connection : scrollContainer.getCallbacks())
	{
		ScrollCallback callback = connection.get()->getCallback();
		callback(scrollDiff);
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
	return MouseOffset(frameMouseXOffset, frameMouseYOffset);
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

float Input::getFrameScrollOffset()
{
	return frameScrollOffset;
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