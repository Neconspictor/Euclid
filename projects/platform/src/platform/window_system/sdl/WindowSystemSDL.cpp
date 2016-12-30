#include <platform/window_system/sdl/WindowSystemSDL.hpp>

using namespace std;

WindowSystemSDL WindowSystemSDL::instance;

Window* WindowSystemSDL::createWindow(Window::WindowStruct& desc, Renderer& renderer)
{
	WindowSDL window = WindowSDL(desc, renderer);
	windows.push_back(move(window));
	WindowSDL* pointer = &windows.back();
	windowsIdMap.insert(make_pair(SDL_GetWindowID(window.window), pointer));
	return pointer;
}

void WindowSystemSDL::destroyWindow(WindowSDL* window)
{
	int id = SDL_GetWindowID(window->window);
	bool foundWindow = false;
	auto it = windowsIdMap.find(id);
	if (it == windowsIdMap.end()) 
		throw runtime_error("WindowSystemSDL::closeWindow(WindowSDL*) : id wasn't found!");

	for (auto it2 = windows.begin(); it2 != windows.end(); ++it2)
	{
		if (&(*it2) == window) {
			foundWindow = true;
			windows.erase(it2);
			break;
		}
	}

	if (!foundWindow)
		throw runtime_error("WindowSystemSDL::closeWindow(WindowSDL*) : WindowSDL isn't registered!");

	windowsIdMap.erase(it);
}

bool WindowSystemSDL::init()
{
	// TODO shift SDL to main initialization
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		return false;
	return true;
}

void WindowSystemSDL::pollEvents()
{
	SDL_PumpEvents();
	SDL_Event event;

	vector<InputDeviceSDL*> queue;

	while(SDL_PollEvent(&event))
	{
		auto it = windowsIdMap.find(event.window.windowID);
		if (it != windowsIdMap.end())
		{
			WindowSDL* window = it->second;
			window->getInputDeviceSDL()->push(move(event));
			queue.push_back(window->getInputDeviceSDL());
		}
	}

	for (InputDeviceSDL* input : queue)
	{
		input->handleEvents();
	}
}

void WindowSystemSDL::terminate()
{
	for (WindowSDL& window : windows)
	{
		SDL_DestroyWindow(window.window);
	}

	// TODO shift SDL to main deinitialization
	SDL_Quit();
}

WindowSystemSDL* WindowSystemSDL::get()
{
	return &instance;
}