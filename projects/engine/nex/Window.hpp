#pragma once
#include <nex/Input.hpp>
#include <nex/gui/Drawable.hpp>

namespace nex
{

	class WindowFocusListener;
	class Input;

	/**
	 * A window is a render target where a renderer executes draw calls on.
	 * A renderer can be associated to several windows, but for each draw call only one window
	 * can be active. A window provides therefore an interface for deciding whether the current window
	 * is active for draw calls or not.
	 *
	 * NOTE: A Window depends on certain platforms on a rendering context. The context has to be created/bound
	 * on window construction -> renderer is needed for constructing a window.
	 */
	class Window
	{
	public:

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
			bool vSync;

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
				vSync = false;
			};
		};

		/**
		 * Creates a new window based on a description object.
		 */
		Window(WindowStruct const& description);

		virtual ~Window() {}

		/**
		* Activates this window. All drawing calls of an registered renderer are going to this window as long
		* as it is active.
		*/
		virtual void activate() = 0;

		/**
		* Closes this window.
		*/
		virtual void close() = 0;

		/**
		* Provides access to the underlying input device.
		* With that device input actions can be queried from the user.
		*/
		virtual Input* getInputDevice() = 0;

		int getHeight() const;
		int getPosX() const;
		int getPosY() const;
		int getWidth() const;

		/**
		 * Provides access to the underlying native window.
		 * Null will be returned if the window implementation doesn't support native window access.
		 */
		virtual void* getNativeWindow() { return nullptr; }

		/**
		 * Provides the title of thi window.
		 */
		const std::string& getTitle() const;

		virtual bool getVsync() const;

		/**
		* Checks, if this window is on the foreground, i.e. it is able to receive input events.
		*/
		virtual bool hasFocus() = 0;

		/**
		* Checks if this window is still open.
		*/
		virtual bool isOpen() = 0;

		/**
		 * Checks if this window is in fullscreen mode.
		 */
		virtual bool isInFullscreenMode();

		/**
		 * Minimizes this window
		 */
		virtual void minimize() = 0;

		/**
		 * If the window is closed but this function opens the window again.
		 */
		virtual void reopen() = 0;

		/**
		* Updates the width and height of this window.
		*/
		virtual void resize(int newWidth, int newHeight) = 0;

		/**
		 * Sets the cursor to a specified position (xPos, yPos)
		 * inside this window. Thereby the position (0,0) is the upper
		 * left corner and (window-width, window-height) is the lower
		 * right corner.
		 */
		virtual void setCursorPosition(int xPos, int yPos) = 0;

		/**
		 * Sets this window to fullscreen mode.
		 * As monitor resolution are used the width, height and colorbit depth, this window currently has.
		 */
		virtual void setFullscreen() = 0;

		/**
		 * Sets the title of this window. The title will only be visible, if this
		 * window has a border and isn't in fullscreen mode.
		 */
		virtual void setTitle(const std::string& newTitle);

		/**
		* Shows or hides a window.
		*/
		virtual void setVisible(bool visible) = 0;

		virtual void setVsync(bool vsync);

		/**
		 * Sets the mode of this window to windowed. If this window was previously in windowed mode, the monitor
		 * resolution will be restored to the width, height and colorbit depth, the monitor had had, before this
		 * window was set to fullscreen mode.
		 */
		virtual void setWindowed() = 0;

		/**
		 * Specifies , whether the cursor should be displayed in this window
		 */
		virtual void showCursor(bool show) = 0;

		/**
		 * Swaps the buffers of the graphics card, the content of this Window is send to.
		 */
		virtual void swapBuffers() = 0;

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
		* Is this window currently focused for input events?
		*/
		bool m_hasFocus;

		/**
		 * Checks if window buffer swapping should be synchronized to monitor refresh rate.
		 */
		bool vSync;

		/**
		 * a logging client for logging internals.
		 */
		nex::Logger m_logger;
	};


	class Window_ConfigurationView : public nex::gui::Drawable {
	public:
		Window_ConfigurationView(Window* window);

	protected:
		void drawSelf() override;

	private:
		Window * m_window;
	};
}