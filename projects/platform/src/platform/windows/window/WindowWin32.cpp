#include "platform/windows/window/WindowWin32.hpp"
#include <iostream>
#include <platform/windows/PlatformWindows.hpp>
#include <platform/windows/input/gainput/WindowsInputDevice.hpp>
#include "platform/windows/input/OIS/OISWindowsID.hpp"
#include <platform/windows/input/SDL/SDLInputDevice.hpp>

using namespace std;

map<HWND, Window*> WindowWin32::windowTable;

WindowWin32::WindowWin32(WindowStruct const& desc):Window(desc), hwnd(nullptr)
{
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

	//if (fullscreen) WindowWin32::setFullscreen();
	//inputDevice = new WindowsInputDevice(width, height);
	//inputDevice = new OISWindowsID(hwnd, width, height);
	//inputDeviceTest = new WindowsInputDevice(width, height);
	sdlInputDevice = new SDLInputDevice(hwnd, width, height);
	if (!((SDLInputDevice*)sdlInputDevice)->isInit())
	{
		cerr << "WindowWin32::WindowWin32: ERROR: Couldn't init sdl input device!" << endl;
		delete sdlInputDevice;
		sdlInputDevice = nullptr;
	}

	// important: do fullscreening at the end for avoiding unintended side effects!
	// E.g. SDL has problems to do resizing a window that was previously in fullscreen mode
	// and was set later to windowed mode!
	if (fullscreen) setFullscreen();
	update();
}

void WindowWin32::embedOpenGLRenderer(Renderer* renderer)
{
	cout << "WindowWin32::embedRenderer(Renderer*): embed openGL renderer..." << endl;
	//PlatformWindows::setOpenGLPixelFormat(hdc);
	if (!openglContext)
		openglContext = PlatformWindows::createOpenGLContext(hdc);
	renderer->init();
	this->renderer = renderer;
}

WindowWin32::~WindowWin32()
{
	cout << "WindowWin32::~WindowWin32() called." << endl;
	
	if (openglContext)
	{
		PlatformWindows::destroyOpenGLContext(openglContext);
		openglContext = nullptr;
		cout << "WindowWin32::~WindowWin32(): destroyed openglContext." << endl;
	}

	if (sdlInputDevice)
	{
		delete sdlInputDevice;
		sdlInputDevice = nullptr;
		cout << "WindowWin32::~WindowWin32(): destroyed sdlInputDevice." << endl;
	}
	
	if (hwnd)
	{
		DestroyWindow(hwnd);
		hwnd = nullptr;
		cout << "WindowWin32::~WindowWin32(): destroyed hwnd." << endl;
	}

	if (hdc)
	{
		ReleaseDC(hwnd, hdc);
		hdc = nullptr;
		cout << "WindowWin32::~WindowWin32(): destroyed hdc." << endl;
	}
}

void WindowWin32::embedRenderer(Renderer* renderer)
{
	RendererType type = renderer->getType();
	switch(type)
	{
	case OPENGL: {
		embedOpenGLRenderer(renderer);
		break;
	}
	default: {
		cout << "WindowWin32::embedRenderer(Renderer*): unsupported Renderer type: " << type << endl;
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

	EnumDisplaySettings(NULL, 0, &fullscreenSettings);
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
		// TODO ERROR log
		cout << "WindowWin32::setFullscreen(): Couldn't change display settings." << endl;
	}
}

void WindowWin32::setWindowed()
{
	fullscreen = false;
	DEVMODE fullscreenSettings;
	bool isChangeSuccessful;

	SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_LEFT);
	SetWindowLongPtr(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_THICKFRAME);
	isChangeSuccessful = ChangeDisplaySettings(NULL, CDS_RESET) == DISP_CHANGE_SUCCESSFUL;
	//SetWindowPos(hwnd, HWND_NOTOPMOST, posX, posY, width, height, SWP_SHOWWINDOW);
	//ShowWindow(hwnd, SW_RESTORE);
	if (isChangeSuccessful)
	{
		//update();
		cout << "WindowWin32::setWindowed(): Change was successful!" << endl;
		update();
	} else
	{
		// TODO ERROR log	
		cerr << "WindowWin32::setWindowed(): Couldn't change!" << endl;
	}
}

void WindowWin32::resize(int newWidth, int newHeight)
{
	width = newWidth;
	height = newHeight;
	update();
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
	//inputDevice->update();
	//inputDeviceTest->update();
	while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
	{
		//inputDeviceTest->handleMessage(msg);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	((SDLInputDevice*)sdlInputDevice)->pollEvents();
}

void WindowWin32::swapBuffers()
{
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
	//return inputDevice;
	//return inputDeviceTest;
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
	ex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	ex.hCursor = LoadCursor(NULL, IDC_ARROW);
	ex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	ex.lpszMenuName = NULL;
	ex.lpszClassName = WNDCLASSNAME.c_str();
	ex.hIconSm = NULL;

	RegisterClassEx(&ex);

	// center position of the window
	int posx = (GetSystemMetrics(SM_CXSCREEN) / 2) - (width / 2);
	int posy = (GetSystemMetrics(SM_CYSCREEN) / 2) - (height / 2);

	// set up the window for a windowed application by default
	long wndStyle = WS_POPUP | WS_OVERLAPPEDWINDOW;

	if (fullscreen)	// create a full-screen application if requested
	{
		//wndStyle = WS_POPUP;
	}

	// create the window
	hwnd = CreateWindowEx(NULL,
		WNDCLASSNAME.c_str(),
		title.c_str(),
		wndStyle | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		posX, posY,
		width, height,
		NULL,
		NULL,
		hinst,
		NULL);

	return hwnd;
}

LRESULT WindowWin32::dispatchInputEvents(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{

	/*case WM_CREATE:
	{
		if ((hdc = GetDC(hwnd)) == NULL)	// get device context
		{
			MessageBox(hwnd, "Failed to Get the Window Device Context",
				"Device Context Error", MB_OK);
			SysShutdown();
			break;
		}
		SetGLFormat();			// select pixel format
		break;
	}*/
	case WM_QUIT: // do the same as for WM_CLOSE{
	{
		Window* target = getWindowByHWND(hwnd);
		target->close();
		break;
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

void WindowWin32::update()
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
			//SetWindowPos(hwnd, HWND_NOTOPMOST, posX, posY, width, height, SWP_SHOWWINDOW);
			ShowWindow(hwnd, SW_SHOW);
		}
	} else
	{
		ShowWindow(hwnd, SW_HIDE);
	}

	UpdateWindow(hwnd);
}