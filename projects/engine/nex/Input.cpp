#include <nex/Input.hpp>


namespace nex
{

	Input::Input() :
		m_logger("Input")
	{
		frameMouseXOffset = frameMouseYOffset = 0;
		frameScrollOffsetX = 0;
		frameScrollOffsetY = 0;
		mouseXabsolut = 0;
		mouseYabsolut = 0;
		firstMouseInput = true;
		m_windowHasFocus = true;
	}

	Input::~Input()
	{
	}

	Input::WindowCloseCallbacks::Handle Input::addWindowCloseCallback(const WindowCloseCallbacks::Callback& callback)
	{
		return m_windowCloseCallbacks.addCallback(callback);
	}

	Input::WindowResizeCallbacks::Handle Input::addResizeCallback(const WindowResizeCallbacks::Callback& callback)
	{
		return windowResizeContainer.addCallback(callback);
	}

	Input::ScrollCallbacks::Handle Input::addScrollCallback(const ScrollCallbacks::Callback& callback)
	{
		return scrollContainer.addCallback(callback);
	}

	Input::WindowFocusCallbacks::Handle Input::addWindowFocusCallback(const WindowFocusCallbacks::Callback& callback)
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

	void Input::informScrollListeners(float scrollX, float scrollY)
	{
		for (const auto& handle : scrollContainer.getCallbacks())
		{
			const auto& callback = *handle;
			callback(scrollX, scrollY);
		}
	}

	void Input::informResizeListeners(int width, int height)
	{
		for (const auto& handle : windowResizeContainer.getCallbacks())
		{
			const auto& callback = *handle;
			callback(width, height);
		}
	}

	void Input::informWindowFocusListeners(bool receivedFocus)
	{
		for (const auto& handle : windowFocusChanged.getCallbacks())
		{
			const auto& callback = *handle;
			callback(getWindow(), receivedFocus);
		}
	}

	void Input::informWindowCloseListeners()
	{
		for (const auto& handle : m_windowCloseCallbacks.getCallbacks())
		{
			const auto callback = *handle;
			callback(getWindow());
		}
	}


	void Input::removeScrollConnection(const ScrollCallbacks::Handle& handle)
	{
		scrollContainer.removeCallback(handle);
	}

	void Input::removeWindowCloseCallback(const WindowCloseCallbacks::Handle& handle)
	{
		m_windowCloseCallbacks.removeCallback(handle);
	}

	void Input::removeResizeCallback(const WindowResizeCallbacks::Handle& handle)
	{
		windowResizeContainer.removeCallback(handle);
	}

	void Input::removeWindowFocusCallback(const WindowFocusCallbacks::Handle& handle)
	{
		windowFocusChanged.removeCallback(handle);
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
}