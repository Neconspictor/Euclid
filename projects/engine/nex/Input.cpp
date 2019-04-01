#include <nex/Input.hpp>


namespace nex
{

	Input::Input() :
		mLogger("Input")
	{
		mFrameScrollOffsetX = 0;
		mFrameScrollOffsetY = 0;
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

	CallbackCollection<void(Input::Button, Input::InputItemState, int)>::Handle Input::addMouseCallback(
		const MouseCallbacks::Callback& callback)
	{
		return mMouseCallbacks.addCallback(callback);
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


	const MouseOffset& Input::getFrameMouseOffset() const
	{
		return mMouseData;
	}


	double Input::getFrameScrollOffsetX() const
	{
		return mFrameScrollOffsetX;
	}

	double Input::getFrameScrollOffsetY() const
	{
		return mFrameScrollOffsetY;
	}

	void Input::informMouseListeners(Input::Button button, Input::InputItemState state, int mods)
	{
		for (const auto& handle : mMouseCallbacks.getCallbacks())
		{
			const auto& callback = *handle;
			callback(button, state, mods);
		}
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


	void Input::removeMouseCallback(const MouseCallbacks::Handle& handle)
	{
		mMouseCallbacks.removeCallback(handle);
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
		mMouseData.xOffset = 0;
		mMouseData.yOffset = 0;
	}

	void Input::setMousePosition(int xPos, int yPos, bool updateOffsets)
	{
		if (updateOffsets)
		{
			mMouseData.xOffset += xPos - mMouseData.xAbsolute;
			mMouseData.yOffset += yPos - mMouseData.yAbsolute;
		}

		mMouseData.xAbsolute = xPos;
		mMouseData.yAbsolute = yPos;
	}

	bool Input::windowHasFocus() const
	{
		return mHasFocus;
	}
}