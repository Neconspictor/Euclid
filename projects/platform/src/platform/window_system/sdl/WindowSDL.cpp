#include <platform/window_system/sdl/WindowSDL.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <platform/window_system/sdl/WindowSystemSDL.hpp>
#include <glad/glad.h>

using namespace platform;

WindowSDL::WindowSDL(WindowStruct const& desc, Renderer& renderer) : Window(desc, renderer),
logClient(getLogServer())
{

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	if (renderer.getType() == OPENGL)
	{
		createOpenGLWindow(desc);
	} else
	{
		createNoAPIWindow(desc);
	}

	closed = false;
}

void WindowSDL::createOpenGLWindow(WindowStruct const& desc)
{
	window = SDL_CreateWindow(desc.title.c_str(), 10, 30, desc.width, desc.height,
		 SDL_WINDOW_RESIZABLE | SDL_WINDOW_UTILITY);

	int x, y, w, h;

	SDL_GetWindowPosition(window, &x, &y);
	SDL_GetWindowSize(window, &w, &h);

	LOG(logClient, Debug) << "x: " << x << ", y: " << y;
	LOG(logClient, Debug) << "w: " << w << ", h: " << h;


	SDL_SetWindowBordered(window, SDL_TRUE);

	SDL_GetWindowPosition(window, &x, &y);
	SDL_GetWindowSize(window, &w, &h);

	LOG(logClient, Debug) << "x: " << x << ", y: " << y;
	LOG(logClient, Debug) << "w: " << w << ", h: " << h;

	//SDL_GL_CreateContext(window);
}

void WindowSDL::createNoAPIWindow(WindowStruct const& desc)
{
	window = SDL_CreateWindow(desc.title.c_str(), desc.posX, desc.posY, desc.width, desc.height, 0);
}

void WindowSDL::activate()
{
	//TODO
}

void WindowSDL::close()
{
	//TODO
	closed = true;
}

Input* WindowSDL::getInputDevice()
{
	return &inputDevice;
}

InputDeviceSDL* WindowSDL::getInputDeviceSDL()
{
	return &inputDevice;
}

bool WindowSDL::hasFocus()
{
	//TODO
	return false;
}

bool WindowSDL::isOpen()
{
	return !closed;
}

void WindowSDL::minimize()
{
	//TODO
}

void WindowSDL::resize(int newWidth, int newHeight)
{
	//TODO
}

void WindowSDL::setCursorPosition(int xPos, int yPos)
{
	//TODO
}

void WindowSDL::setFullscreen()
{
	//TODO
}

void WindowSDL::setVisible(bool visible)
{
	//TODO
}

void WindowSDL::setWindowed()
{
	//TODO
}

void WindowSDL::swapBuffers()
{
	//TODO
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	SDL_GL_SwapWindow(window);
}