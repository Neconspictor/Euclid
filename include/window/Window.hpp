#ifndef WINDOW_HPP
#define WINDOW_HPP

#include "renderer/Renderer.hpp"
#include <string>
#include "input/Input.hpp"
#include <vector>
#include <functional>
#include <util/CallbackContainer.hpp>
#include <util/Signal.hpp>

class WindowFocusListener;

/**
 * A window is a render target where a renderer executes draw calls on. 
 * A renderer can be associated to several windows, but for each draw call only one window 
 * can be active. A window provides therefore an interface for deciding whether the current window
 * is active for draw calls or not.
 */
class Window
{
private:

	//using  WindowFocusChanged = Signal<Window*, bool>;
	using WindowFocusChanged = CallbackContainer<void(Window*, bool)>;
	WindowFocusChanged windowFocusChanged;

public:

	//using WindowFocusCallback = WindowFocusChanged::Callback;
	using WindowFocusCallback = WindowFocusChanged::Callback;
	//using WindowFocusConnection = WindowFocusChanged::Connection;
	using WindowFocusConnection = WindowFocusChanged::SharedItem;
	//using WindowFocusScopedConnection = WindowFocusChanged::ScopedConnection;

	//using WindowFocusCallbackType = void(Window* window, bool hasFocus);
	//using WindowFocusCallback = std::function<WindowFocusCallbackType>;

	virtual ~Window(){}

	/**
	 * Holds configuration properties used to customize a window.
	 */
	struct WindowStruct
	{
		int width;
		int height;
		int colorBitDepth;
		bool fullscreen;
		int refreshRate;
		int posX;
		int posY;
		std::string title;
		bool visible;

		WindowStruct()
		{
			width = 0;
			height = 0;
			colorBitDepth = 0;
			fullscreen = false;
			refreshRate = 0;
			posX = 0;
			posY = 0;
			title = "";
			visible = false;
		};
	};

	/**
	 * Creates a new window based on a description object.
	 */
	Window(WindowStruct const& description);

	/**
	 * Embeds the specified renderer. 
	 * All draw calls of the renderer will be presented on this window.
	 */
	virtual void embedRenderer(Renderer* renderer) = 0;

	/**
	 * Shows or hides a window.
	 */
	virtual void setVisible(bool visible) = 0;

	/**
	 * Minimizes this window 
	 */
	virtual void minimize() = 0;

	/**
	 * Sets this window to fullscreen mode.
	 * As monitor resolution are used the width, height and colorbit depth, this window currently has.
	 */
	virtual void setFullscreen() = 0;

	/**
	 * Sets the mode of this window to windowed. If this window was previously in windowed mode, the monitor
	 * resolution will be restored to the width, height and colorbit depth, the monitor had had, before this 
	 * window was set to fullscreen mode.
	 */
	virtual void setWindowed() = 0;


	/**
	 * Updates the width and height of this window.
	 */
	virtual void resize(int newWidth, int newHeight) = 0;

	/**
	 * Checks if this window is still open.
	 */
	virtual bool isOpen() = 0;

	/**
	 * Closes this window and releases any allocated memory.
	 */
	virtual void close() = 0;

	/**
	 * Polls and process events for this window.
	 */
	virtual void pollEvents() = 0;

	/**
	 * Swaps the buffers of the graphics card, the content of this Window is send to.
	 */
	virtual void swapBuffers() = 0;

	/**
	 * Activates this window. All drawing calls of an registered renderer are going to this window as long
	 * as it is active.
	 */
	virtual void activate() = 0;

	/**
	 * Provides access to the underlying input device. 
	 * With that device input actions can be queried from the user.
	 */
	virtual Input* getInputDevice() = 0;

	/**
	 * Checks, if this window is on the foreground, i.e. it is able to receive input events.
	 */
	virtual bool hasFocus() = 0;

	WindowFocusConnection addWindowFocusCallback(const WindowFocusCallback& callback);
	void removeWindowFocusCallback(const WindowFocusConnection& connection);

protected:
	/**
	* The width of this window.
	*/
	int width;

	/**
	* The height of this window.
	*/
	int height;

	/**
	* the color depth used on the monitor, measured in bits.
	* Usual values are 16, 24 and 32 bits.
	*/
	int colorBitDepth;

	/**
	* Specifies if the window is drawn in fullscreen mode or in windowed mode.
	*/
	bool fullscreen;

	/**
	* Specifies if the window is currently open.
	*/
	bool m_isOpen;

	/**
	* Refresh rate the monitor should use, if the window is in fullscreen mode.
	*/
	int refreshRate;

	/**
	* Specifies if the window is currently visible or hidden.
	*/
	bool m_isVisible;

	/**
	* The position of the window on screen
	*/
	int posX, posY;

	/**
	* The title of the window.
	*/
	std::string title;

	/**
	* The renderer that is associated with this window (if any renderer is associated at all).
	*/
	Renderer* renderer;

	/**
	* Is this window currently focused for input events?
	*/
	bool m_hasFocus;

	//std::vector<WindowFocusCallback> windowFocusCallbacks;
	//CallbackContainer<WindowFocusCallbackType> windowFocusCallbackContainer;


	void informWindowFocusListeners(bool receivedFocus);
};

#endif