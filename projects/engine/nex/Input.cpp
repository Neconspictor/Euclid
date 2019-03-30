#include <nex/Input.hpp>


namespace nex
{

	Input::Input() :
		mLogger("Input")
	{
		mFrameMouseXOffset = mFrameMouseYOffset = 0;
		mFrameScrollOffsetX = 0;
		mFrameScrollOffsetY = 0;
		mMouseXabsolut = 0;
		mMouseYabsolut = 0;
		mFirstMouseInput = true;
		mHasFocus = true;
	}

	Input::~Input()
	{
	}

	CallbackCollection<void(unsigned, unsigned)>::Handle Input::addFrameBufferResizeCallback(
		const FrameBufferResizeCallbacks::Callback& callback)
	{
		return mFrameBufferResizeContainer.addCallback(callback);
	}

	Input::CloseCallbacks::Handle Input::addWindowCloseCallback(const CloseCallbacks::Callback& callback)
	{
		return mCloseCallbacks.addCallback(callback);
	}

	Input::VirtualDimesionResizeCallbacks::Handle Input::addVirtualDimensionResizeCallback(const VirtualDimesionResizeCallbacks::Callback& callback)
	{
		return mVirtualDimensionResizeContainer.addCallback(callback);
	}

	Input::ScrollCallbacks::Handle Input::addScrollCallback(const ScrollCallbacks::Callback& callback)
	{
		return mScrollContainer.addCallback(callback);
	}

	Input::FocusCallbacks::Handle Input::addWindowFocusCallback(const FocusCallbacks::Callback& callback)
	{
		return mFocusChanged.addCallback(callback);
	}


	MouseOffset Input::getFrameMouseOffset()
	{
		MouseOffset offset;
		offset.xAbsolute = mMouseXabsolut;
		offset.yAbsolute = mMouseYabsolut;
		offset.xOffset = mFrameMouseXOffset;
		offset.yOffset = mFrameMouseYOffset;
		return offset;
	}


	double Input::getFrameScrollOffsetX()
	{
		return mFrameScrollOffsetX;
	}

	double Input::getFrameScrollOffsetY()
	{
		return mFrameScrollOffsetY;
	}

	void Input::informScrollListeners(float scrollX, float scrollY)
	{
		for (const auto& handle : mScrollContainer.getCallbacks())
		{
			const auto& callback = *handle;
			callback(scrollX, scrollY);
		}
	}

	void Input::informVirtualDimensionResizeListeners(unsigned width, unsigned height)
	{
		for (const auto& handle : mVirtualDimensionResizeContainer.getCallbacks())
		{
			const auto& callback = *handle;
			callback(width, height);
		}
	}

	void Input::inforrmFrameBufferResiteListeners(unsigned frameBufferWidth, unsigned frameBufferHeight)
	{
		for (const auto& handle : mFrameBufferResizeContainer.getCallbacks())
		{
			const auto& callback = *handle;
			callback(frameBufferWidth, frameBufferHeight);
		}
	}

	void Input::informWindowFocusListeners(bool receivedFocus)
	{
		for (const auto& handle : mFocusChanged.getCallbacks())
		{
			const auto& callback = *handle;
			callback(getWindow(), receivedFocus);
		}
	}

	void Input::informWindowCloseListeners()
	{
		for (const auto& handle : mCloseCallbacks.getCallbacks())
		{
			const auto callback = *handle;
			callback(getWindow());
		}
	}


	void Input::removeScrollConnection(const ScrollCallbacks::Handle& handle)
	{
		mScrollContainer.removeCallback(handle);
	}

	void Input::removeWindowCloseCallback(const CloseCallbacks::Handle& handle)
	{
		mCloseCallbacks.removeCallback(handle);
	}

	void Input::removeResizeCallback(const VirtualDimesionResizeCallbacks::Handle& handle)
	{
		mVirtualDimensionResizeContainer.removeCallback(handle);
	}

	void Input::removeWindowFocusCallback(const FocusCallbacks::Handle& handle)
	{
		mFocusChanged.removeCallback(handle);
	}

	void Input::resetMouseMovement()
	{
		mFrameMouseXOffset = 0;
		mFrameMouseYOffset = 0;
	}

	void Input::setMousePosition(int xPos, int yPos)
	{
		mFrameMouseXOffset += xPos - mMouseXabsolut;
		mFrameMouseYOffset += yPos - mMouseYabsolut;
		mMouseXabsolut = xPos;
		mMouseYabsolut = yPos;
	}

	bool Input::windowHasFocus()
	{
		return mHasFocus;
	}
}