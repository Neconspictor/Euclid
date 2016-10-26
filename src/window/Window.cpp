#include "window/Window.hpp"

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
	//windowFocusCallbacks.push_back(callback);
	//return windowFocusCallbackContainer.addCallback(callback);
	//return windowFocusChanged.connect(callback);
	return windowFocusChanged.addCallback(callback);
}

void Window::removeWindowFocusCallback(const WindowFocusConnection& connection)
{
	windowFocusChanged.removeCallback(connection);
}

/*void Window::removeWindowFocusCallback(unsigned int key)
{
	/*auto it = find_if(windowFocusCallbacks.begin(), windowFocusCallbacks.end(), [&](const WindowFocusCallback& current) {
		
		auto ptr1 = current.target<CallbackType>();
		auto ptr2 = callback.target<CallbackType>();
		return ptr1 == ptr2;
	});
	if (it != windowFocusCallbacks.end())
		windowFocusCallbacks.erase(it);*/
/*	windowFocusCallbackContainer.removeCallback(key);
}*/

void Window::informWindowFocusListeners(bool receivedFocus)
{
	/*for (std::pair<unsigned int, WindowFocusCallback> current : windowFocusCallbackContainer.getCallbacks())
	{
		current.second(this, receivedFocus);
	}*/
	//windowFocusChanged(this, receivedFocus);

	for (WindowFocusChanged::SharedItem sharedItem : windowFocusChanged.getCallbacks())
	{
		WindowFocusChanged::Callback callback = sharedItem.get()->getCallback();
		callback(this, receivedFocus);
	}
}