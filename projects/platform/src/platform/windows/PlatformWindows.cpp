#include <platform/windows/PlatformWindows.hpp>
#include <platform/windows/window/WindowWin32.hpp>
#include <GL/glew.h>
#include <GL/wglew.h>
#include <platform/logging/LoggingClient.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <platform/exception/UnexpectedPlatformException.hpp>
#include <platform/exception/OpenglException.hpp>

using namespace std;

PlatformWindows::PlatformWindows() : logClient(platform::getLogServer())
{
	logClient.setPrefix("[PlatformWindows]");
}

void PlatformWindows::setVSync(const Renderer& renderer, bool value)
{
	switch(renderer.getType())
	{
	case OPENGL: {
		setVSyncOpenGL(value);
		break;
	}
	case DIRECTX: {
		LOG(logClient, platform::Error) << "Renderer type currently not supported: " << DIRECTX;
		break; 
	}
	default: break;
	}
}

unique_ptr<Window> PlatformWindows::createWindow(Window::WindowStruct const& desc)
{
	return make_unique<WindowWin32>(desc);
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
		MessageBox(nullptr, "GLEW is not initialized!",
			"OpenGL Rendering Context Error", MB_OK);
	}

	int desiredOpenGLVersion[2];
	desiredOpenGLVersion[0] = 4;
	desiredOpenGLVersion[1] = 5;

	int numVersions = 0;
	glGetIntegerv(GL_NUM_SHADING_LANGUAGE_VERSIONS, &numVersions);

	for (int i = 0; i < numVersions; ++i)
	{
		const GLubyte* version = glGetStringi(GL_SHADING_LANGUAGE_VERSION, i);
		if (version)
		{
			LOG(logClient, platform::Debug) << version;
		}
		else 
			LOG(logClient, platform::Debug) << "is not supported: " << i;
	}

	LOG(logClient, platform::Debug) << "Expected OpenGL version: " << desiredOpenGLVersion[0] << "." << desiredOpenGLVersion[1];

	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, desiredOpenGLVersion[0],
		WGL_CONTEXT_MINOR_VERSION_ARB, desiredOpenGLVersion[1],
		WGL_CONTEXT_LAYER_PLANE_ARB, 0, // default value 
		WGL_CONTEXT_FLAGS_ARB, 0, // default value
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB, // only core profile - no deprecated functionality
		0, // end of parameters
	};


	//Check OpenGL version retrieved
	int OpenGLVersion[2];
	glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersion[1]);

	// Couldn't retrieve an opengl context
	if (desiredOpenGLVersion[0] != OpenGLVersion[0] 
		|| desiredOpenGLVersion[1] != OpenGLVersion[1])
	{
		stringstream ss; ss << "PlatformWindows::createOpenGLContext(HDC& hdc): This OpenGL version isn't supported: " << desiredOpenGLVersion[0]
			<< "." << desiredOpenGLVersion[1];
		throw OpenglException(ss.str());
	}

	LOG(logClient, platform::Debug) << "Used OpenGL version: " << OpenGLVersion[0] << "." << OpenGLVersion[1];
	LOG(logClient, platform::Debug) << "OpenGL version old: " << glGetString(GL_VERSION);

	if (!wglewIsSupported("WGL_ARB_create_context"))
	{
		throw OpenglException("PlatformWindows::createOpenGLContext(HDC& hdc): WGL_ARB_create_context extension isn't supported!");
	}


	hglrc = wglCreateContextAttribsARB(hdc, 0, attribs);
	if (!hglrc)
	{
		LOG(logClient, platform::Error) << GetLastErrorAsString();
		throw OpenglException("PlatformWindows::createOpenGLContext(HDC& hdc): OpenGL Context couldn't be created!");
	}
	destroyOpenGLContext(tempContext);
	wglMakeCurrent(hdc, hglrc);

	GLint error = glGetError();
	if (error != GL_NO_ERROR)
	{
		throw OpenglException("PlatformWindows::createOpenGLContext(HDC& hdc): Error occured during init!");
	}

	return hglrc;
}

void PlatformWindows::destroyOpenGLContext(HGLRC hglrc)
{
	wglMakeCurrent(nullptr, nullptr);
	wglDeleteContext(hglrc);
}

PlatformWindows* PlatformWindows::getActivePlatform()
{
	PlatformWindows* platform = dynamic_cast<PlatformWindows*> (Platform::getActivePlatform().get());
	if (!platform)
	{
		throw UnexpectedPlatformException("PlatformWindows::getActivePlatform(): Expected platform type: PlatformWindows");
	}

	return platform;
}



//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
string PlatformWindows::GetLastErrorAsString()
{
	//Get the error message, if any.
	DWORD errorMessageID = GetLastError();
	if (errorMessageID == 0)
		return string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}



void PlatformWindows::setVSyncOpenGL(bool value)
{
	/*GLint n, i;
	glGetIntegerv(GL_NUM_EXTENSIONS, &n);
	for (i = 0; i < n; i++) {
		cout << glGetStringi(GL_EXTENSIONS, i) << endl;
	}*/

	typedef BOOL(APIENTRY *PFNWGLSWAPINTERVALFARPROC)(int);
	PFNWGLSWAPINTERVALFARPROC wglSwapIntervalEXT = 0;

	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress("wglSwapIntervalEXT");

	if (!wglSwapIntervalEXT)
		throw OpenglException("PlatformWindows::setVSyncOpenGL(bool value): WGL_EXT_swap_control extension not supported.");
	wglSwapIntervalEXT(value);
}
