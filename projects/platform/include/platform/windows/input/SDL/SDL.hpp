#pragma once
#include <SDL/SDL.h>
#include <platform/windows/input/SDL/SDLInitException.hpp>

/**
* A class that creates and manages an SDL library context specific for window handling.
*/
class SDL {
public:

	/**
	 * Creates a new sdl context and initializes it with given flags.
	 * NOTE: A SDLInitError is thrown if the SDL object couldn't be initialized successfully.
	 */
	explicit SDL(Uint32 flags = 0) : window(nullptr)
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
	};

	/**
	 * Sets the sdl window of this sdl context
	 */
	void setWindow(SDL_Window* window)
	{
		this->window = window;
	}

	/**
	* Provides the sdl window of this sdl context
	*/
	SDL_Window* getWindow() const
	{
		return window;
	}

protected:
	SDL_Window* window;
};