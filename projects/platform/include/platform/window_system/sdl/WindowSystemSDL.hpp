#pragma once

#include <platform/WindowSystem.hpp>
#include <unordered_map>
#include <platform/window_system/sdl/WindowSDL.hpp>

/**
* A class that creates and manages an SDL library context specific for window handling.
*/
class WindowSystemSDL : public WindowSystem {
public:
	Window* createWindow(Window::WindowStruct& desc, Renderer& renderer) override;
	
	void destroyWindow(WindowSDL* window);
	
	bool init() override;
	void pollEvents() override;
	void terminate() override;

	static WindowSystemSDL* get();

	/**
	 * Creates a new sdl context and initializes it with given flags.
	 * NOTE: A SDLInitError is thrown if the SDL object couldn't be initialized successfully.
	 */
	/*explicit SDL(Uint32 flags = 0)
	{
		if (SDL_Init(flags) != 0)
			throw SDLInitError();
	};
	virtual ~SDL()
	{
		if (window)
		{
			SDL_DestroyWindow(window);
			window = nullptr;
		}
		SDL_Quit();
	};*/

protected:

	std::unordered_map<int, WindowSDL*> windowsIdMap;
	std::list<WindowSDL> windows;

	static WindowSystemSDL instance;
};