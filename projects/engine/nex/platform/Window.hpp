#pragma once
#include <nex/common/Log.hpp>
#include <nex/gui/Drawable.hpp>

namespace nex
{

	class WindowFocusListener;
	class Input;
	class Window;

	enum class StandardCursorType
	{
		Arrow, FIRST = Arrow,
		Hand,
		TextIBeam,
		CrossHair,
		HorizontalResize,
		VerticalResize, LAST = VerticalResize
	};

	enum class CursorState
	{
		Disabled, FIRST = Disabled,
		Hidden,
		Normal, LAST = Normal
	};

	class Cursor
	{
	public:
		/**
		 * Creates a cursor with a standard type visual.
		 */
		Cursor(StandardCursorType type);
		~Cursor();

		Cursor(const Cursor&) = delete;
		Cursor(Cursor&&) = delete;
		Cursor& operator=(const Cursor&) = delete;
		Cursor& operator=(Cursor&&) = delete;

		class Impl;

		Impl* getImpl();

	private:
		std::unique_ptr<Impl> mPimpl;
	};

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
			/**
			* The width of the framebuffer (pixel size).
			*/
			unsigned frameBufferWidth;

			/**
			* The height of the framebuffer (pixel size).
			*/
			unsigned frameBufferHeight;

			/**
			* The width of the (virtual) screen. Can be different from the framebuffer width (e.g. retina displays)
			*/
			unsigned virtualScreenWidth;
			
			/**
			* The height of the (virtual) screen. Can be different from the framebuffer height (e.g. retina displays)
			*/
			unsigned virtualScreenHeight;

			/**
			* the color depth used on the monitor, measured in bits.
			* Usual values are 16, 24 and 32 bits.
			*/
			unsigned colorBitDepth;

			/**
			* Specifies if the window is drawn in fullscreen mode or in windowed mode.
			*/
			bool fullscreen;

			/**
			* Refresh rate the monitor should use, if the window is in fullscreen mode.
			*/
			unsigned refreshRate;

			/**
			* The x-position of the window on screen (in virtual screen coodinates)
			*/
			unsigned posX;
			
			/**
			* The y-position of the window on screen (in virtual screen coodinates)
			*/
			unsigned posY;

			/**
			* The title of the window.
			*/
			std::string title;

			/**
			* Specifies if the window is currently visible or hidden.
			*/
			bool visible;

			/**
			 * Specifies if vertical synchronization is active.
			 */
			bool vSync;

			/**
			 * Specifies a Window object render data should be shared (if render backend needs that).
			 * Nullptr is a valid value and means "no sharing".
			 */
			Window* shared;

			WindowStruct() :
				frameBufferWidth(0),
				frameBufferHeight(0),
				virtualScreenWidth(0),
				virtualScreenHeight(0),
				colorBitDepth(0),
				fullscreen(false),
				refreshRate(0),
				posX(0),
				posY(0),
				title(""),
				visible(false),
				vSync(false),
				shared(nullptr)
			{
			}
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
		virtual void activate(bool deactivate=false) = 0;

		/**
		* Closes this window.
		*/
		virtual void close(bool silent = false) = 0;

		/**
		 * Provides the active cursor.
		 */
		virtual const Cursor* getCursor() const = 0;

		virtual CursorState getCursorState() const = 0;

		/**
		* Provides access to the underlying input device.
		* With that device input actions can be queried from the user.
		*/
		virtual Input* getInputDevice() = 0;

		/**
		 * Provides the height of the framebuffer (in pixels).
		 */
		unsigned getFrameBufferHeight() const;
		
		/**
		 * Provides the x-position of the window (in virtual screen coordinates).
		 */
		unsigned getPosX() const;
		
		/**
		 * Provides the y-position of the window (in virtual screen coordinates).
		 */
		unsigned getPosY() const;
		
		/**
		 * Provides the width of the framebuffer (in pixels).
		 */
		unsigned getFrameBufferWidth() const;
		
		/**
		 * Provides the width of the virtual screen.
		 */
		unsigned getVirtualScreenHeight() const;
		
		/**
		 * Provides the height of the virtual screen.
		 */
		unsigned getVirtualScreenWidth() const;

		/**
		 * Provides access to the underlying native window.
		 * Null will be returned if the window implementation doesn't support native window access.
		 */
		virtual void* getNativeWindow() { return nullptr; }

		/**
		 * Provides the title of thi window.
		 */
		const std::string& getTitle() const;

		/**
		 * Checks, if vertical synchronization is currently active.
		 */
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
		virtual void resize(unsigned newWidth, unsigned newHeight) = 0;

		/**
		 * Sets the active cursor.
		 */
		virtual void setCursor(Cursor* cursor) = 0;

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

		/**
		 * Enables/Disables vertical synchronization.
		 */
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
		virtual void showCursor(CursorState state) = 0;

		/**
		 * Swaps the buffers of the graphics card, the content of this Window is send to.
		 */
		virtual void swapBuffers() = 0;

	protected:

		WindowStruct mConfig;
		bool mIsClosed;
		bool mHasFocus;
		nex::Logger mLogger;
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