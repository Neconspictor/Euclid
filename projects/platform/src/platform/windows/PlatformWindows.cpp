#include <platform/windows/PlatformWindows.hpp>
#include <platform/windows/window/WindowWin32.hpp>
#include <GL/glew.h>
#include <GL/wglew.h>

int PlatformWindows::createWindow(Window::WindowStruct const& desc)
{
	return 0;
}

void PlatformWindows::showWindow(int handle, bool showIt)
{
}

void PlatformWindows::destroyWindow(int handle)
{
}

void PlatformWindows::release()
{
}

void PlatformWindows::setOpenGLPixelFormat(HDC& hdc)
{
	// number of available formats
	int indexPixelFormat = 0;

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
		PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
		32,                        //Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,                        //Number of bits for the depthbuffer
		8,                        //Number of bits for the stencilbuffer
		0,                        //Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	// Choose the closest pixel format available
	indexPixelFormat = ChoosePixelFormat(hdc, &pfd);

	// Set the pixel format for the provided window DC
	if (!indexPixelFormat) return;
	SetPixelFormat(hdc, indexPixelFormat, &pfd);
}

HGLRC PlatformWindows::createOpenGLContext(HDC& hdc)
{
	HGLRC hglrc = nullptr;
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 32;
	pfd.iLayerType = PFD_MAIN_PLANE;

	int nPixelFormat = ChoosePixelFormat(hdc, &pfd);

	if (nPixelFormat == 0) return false;

	BOOL bResult = SetPixelFormat(hdc, nPixelFormat, &pfd);

	if (!bResult) return false;

	HGLRC tempContext = wglCreateContext(hdc);
	wglMakeCurrent(hdc, tempContext);

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		MessageBox(NULL, "GLEW is not initialized!",
			"OpenGL Rendering Context Error", MB_OK);
	}

	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 1,
		WGL_CONTEXT_FLAGS_ARB, 0,
		0
	};

	if (wglewIsSupported("WGL_ARB_create_context") == 1)
	{
		hglrc = wglCreateContextAttribsARB(hdc, 0, attribs);
		destroyOpenGLContext(tempContext);
		wglMakeCurrent(hdc, hglrc);
	}
	else
	{	//It's not possible to make a GL 3.x context. Use the old style context (GL 2.1 and before)
		hglrc = tempContext;
	}

	//Checking GL version
	const GLubyte *GLVersionString = glGetString(GL_VERSION);

	//Or better yet, use the GL3 way to get the version number
	int OpenGLVersion[2];
	glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersion[1]);

	return hglrc;
}

void PlatformWindows::destroyOpenGLContext(HGLRC hglrc)
{
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hglrc);
}