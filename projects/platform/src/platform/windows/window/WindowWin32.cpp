#include <platform/windows/window/WindowWin32.hpp>
#include <platform/windows/PlatformWindows.hpp>
#include <Brofiler.h>
#include <platform/exception/UnexpectedPlatformException.hpp>

using namespace std;
using namespace platform;

map<HWND, Window*> WindowWin32::windowTable;

WindowWin32::WindowWin32(WindowStruct const& desc):Window(desc), hwnd(nullptr)
{
	logClient.setPrefix("[WindowWin32]");
	m_isVisible = desc.visible;
	width = desc.width;
	height = desc.height;
	HINSTANCE appInstance = GetModuleHandle(nullptr);
	hwnd = createWindow(appInstance, title);
	hdc = GetDC(hwnd);
	openglContext = nullptr;
	if (!hwnd)
	{
		m_isOpen = false;
	}

	windowTable.insert(pair<HWND, Window*>(hwnd, this));

	sdlInputDevice = new SDLInputDevice(hwnd, width, height);
	if (!sdlInputDevice->isInit())
	{
		LOG(logClient, Error) << "Couldn't init sdl input device!";
		delete sdlInputDevice;
		sdlInputDevice = nullptr;
	}

	// important: do fullscreening at the end for avoiding unintended side effects!
	// E.g. SDL has problems to do resizing a window that was previously in fullscreen mode
	// and was set later to windowed mode!
	if (fullscreen) setFullscreen();

	update();
	LOG(logClient, Debug) << "WindowWin32 successfully created!";

}

void WindowWin32::embedOpenGLRenderer(shared_ptr<Renderer>& renderer)
{
	LOG(logClient, Debug)  << " Embed openGL renderer...";
	if (!openglContext)
	{
		PlatformWindows* platform = PlatformWindows::getActivePlatform();
		openglContext = platform->createOpenGLContext(hdc);
	}
	renderer->setViewPort(0, 0, width, height);
	this->renderer = renderer;
}

WindowWin32::~WindowWin32()
{
	LOG(logClient, Debug) << "WindowWin32::~WindowWin32() called.";
	
	if (openglContext)
	{
		try
		{
			PlatformWindows* platform = PlatformWindows::getActivePlatform();
			platform->destroyOpenGLContext(openglContext);
		} catch (const UnexpectedPlatformException& e)
		{
			LOG(logClient, Error) << "Couldn't destroy opengl context, since platform object isn't of type PlatformWindows!";
		}
		openglContext = nullptr;
	}

	if (sdlInputDevice)
	{
		delete sdlInputDevice;
		sdlInputDevice = nullptr;
	}
	
	if (hwnd)
	{
		DestroyWindow(hwnd);
		hwnd = nullptr;
	}

	if (hdc)
	{
		ReleaseDC(hwnd, hdc);
		hdc = nullptr;
	}
}

void WindowWin32::embedRenderer(shared_ptr<Renderer>& renderer)
{
	RendererType type = renderer->getType();
	switch(type)
	{
	case OPENGL: {
		embedOpenGLRenderer(renderer);
		break;
	}
	default: {
		LOG(logClient, Error) << "Unsupported Renderer type: " << type;
		break;
	}
	}
}

void WindowWin32::setVisible(bool visible)
{
	m_isVisible = visible;
	update();
}

void WindowWin32::minimize()
{
	ShowWindow(hwnd, SW_MINIMIZE);
}

void WindowWin32::setFullscreen()
{
	fullscreen = true;

	DEVMODE fullscreenSettings;
	bool isChangeSuccessful;

	EnumDisplaySettings(nullptr, 0, &fullscreenSettings);
	fullscreenSettings.dmPelsWidth = width;
	fullscreenSettings.dmPelsHeight = height;
	fullscreenSettings.dmBitsPerPel = colorBitDepth;
	fullscreenSettings.dmDisplayFrequency = refreshRate;
	fullscreenSettings.dmFields = DM_PELSWIDTH |
		DM_PELSHEIGHT |
		DM_BITSPERPEL |
		DM_DISPLAYFREQUENCY;

	SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);
	SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, width, height, SWP_SHOWWINDOW);
	isChangeSuccessful = ChangeDisplaySettings(&fullscreenSettings, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL;
	if (isChangeSuccessful)
	{
		update();
	}
	else
	{
		LOG(logClient, Error) << "WindowWin32::setFullscreen(): Couldn't change display settings.";
	}
}

void WindowWin32::setTitle(const string& newTitle)
{
	Window::setTitle(newTitle);
	if(!SetWindowText(hwnd, title.c_str()))
		LOG(logClient, Error) << "setTitle(): Couldn't set window title!";
}

void WindowWin32::setWindowed()
{
	fullscreen = false;
	bool isChangeSuccessful;

	SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_LEFT);
	SetWindowLongPtr(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_THICKFRAME);
	isChangeSuccessful = ChangeDisplaySettings(nullptr, CDS_RESET) == DISP_CHANGE_SUCCESSFUL;

	if (isChangeSuccessful)
	{
		LOG(logClient, Debug) << "WindowWin32::setWindowed(): Change was successful!";
		update();
	} else
	{
		LOG(logClient, Error) << "WindowWin32::setWindowed(): Couldn't change!";
	}
}

void WindowWin32::resize(int newWidth, int newHeight)
{
	width = newWidth;
	height = newHeight;
	update();
}

void WindowWin32::setCursorPosition(int xPos, int yPos)
{
	POINT pos;
	pos.x = xPos;
	pos.y = yPos;

	// map local window coordinates to screen coordinates
	ClientToScreen(hwnd, &pos);
	SetCursorPos(pos.x, pos.y);
	sdlInputDevice->setMousePosition(xPos, yPos);
}

bool WindowWin32::isOpen()
{
	return m_isOpen;
}

void WindowWin32::close()
{
	m_isOpen = false;

	if (hwnd)
	{
		DestroyWindow(hwnd);
		hwnd = nullptr;
	}
	PostQuitMessage(0);
}

void WindowWin32::pollEvents()
{
	MSG msg;
	while (PeekMessage(&msg, nullptr, NULL, NULL, PM_REMOVE))
	{
		//inputDeviceTest->handleMessage(msg);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	sdlInputDevice->pollEvents();
}

void WindowWin32::swapBuffers()
{
	//BROFILER_EVENT("WindowWin32::swapBuffers()");
	SwapBuffers(hdc);
}

void WindowWin32::activate()
{
	if (!renderer) return;
	if (renderer->getType() == OPENGL)
	{
		wglMakeCurrent(hdc, openglContext);
	}
}

Input* WindowWin32::getInputDevice()
{
	return sdlInputDevice;
}

bool WindowWin32::hasFocus()
{
	return m_hasFocus;
}

Window* WindowWin32::getWindowByHWND(HWND hwnd)
{
	auto it = windowTable.find(hwnd);
	if (it == windowTable.end())
	{
		return nullptr;
	}
	return it->second;
}

HWND WindowWin32::createWindow(HINSTANCE& hinst, string title)
{

	static string WNDCLASSNAME = "WindowWin32";
	HWND hwnd;
	WNDCLASSEX ex;

	ex.cbSize = sizeof(WNDCLASSEX);
	ex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	ex.lpfnWndProc = dispatchInputEvents;
	ex.cbClsExtra = 0;
	ex.cbWndExtra = 0;
	ex.hInstance = hinst;
	ex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	ex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	ex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	ex.lpszMenuName = nullptr;
	ex.lpszClassName = WNDCLASSNAME.c_str();
	ex.hIconSm = nullptr;

	RegisterClassEx(&ex);

	// center position of the window
	int posx = (GetSystemMetrics(SM_CXSCREEN) / 2) - (width / 2);
	int posy = (GetSystemMetrics(SM_CYSCREEN) / 2) - (height / 2);

	// set up the window for a windowed application by default
	long wndStyle = WS_POPUP | WS_OVERLAPPEDWINDOW;

	// create the window
	hwnd = CreateWindowEx(NULL,
		WNDCLASSNAME.c_str(),
		title.c_str(),
		wndStyle | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		posX, posY,
		width, height,
		nullptr,
		nullptr,
		hinst,
		nullptr);

	return hwnd;
}

LRESULT WindowWin32::dispatchInputEvents(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{

	case WM_QUIT:
	{
		//do the same for WM_CLOSE !!!
	}
	case WM_CLOSE:
	{
		Window* target = getWindowByHWND(hwnd);
		target->close();
		break;
	}

	case WM_SETFOCUS: {
		WindowWin32* target = (WindowWin32*)getWindowByHWND(hwnd);
		target->m_hasFocus = true;
		target->informWindowFocusListeners(true);
		break;
	}
	case WM_KILLFOCUS: {
		WindowWin32* target = (WindowWin32*)getWindowByHWND(hwnd);
		target->m_hasFocus = false;
		target->informWindowFocusListeners(false);
		break;
	}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void WindowWin32::update() const
{
	if (!hwnd) return;
	if (m_isVisible)
	{
		if (fullscreen)
		{
			RECT screen;
			// Gets the Desktop window
			HWND hDesktop = GetDesktopWindow();
			// Gets the Desktop window rect or screen resolution in pixels
			GetWindowRect(hDesktop, &screen);
			int widthNew = abs(screen.right - screen.left);
			int heightNew = abs(screen.bottom - screen.top);
			SetWindowPos(hwnd, HWND_TOPMOST, screen.left, screen.top, widthNew, heightNew, SWP_SHOWWINDOW);
			ShowWindow(hwnd, SW_SHOWMAXIMIZED);
		} else
		{
			SetWindowPos(hwnd, HWND_NOTOPMOST, posX, posY, width, height, SWP_SHOWWINDOW);
			ShowWindow(hwnd, SW_SHOW);
		}
	} else
	{
		ShowWindow(hwnd, SW_HIDE);
	}

	UpdateWindow(hwnd);
}