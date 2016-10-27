#ifndef SDL_HPP
#define SDL_HPP
#include <SDL/SDL.h>
#include <platform/windows/input/SDL/SDLInitException.hpp>

class SDL {
public:
	explicit SDL(Uint32 flags = 0) throw(InitError)
	{
		if (SDL_Init(flags) != 0)
			throw InitError();
	};
	virtual ~SDL()
	{
		SDL_DestroyWindow(window);
		window = nullptr;
		SDL_Quit();
	};

	void setWindow(SDL_Window* window)
	{
		this->window = window;
	}

	SDL_Window* getWindow() const
	{
		return window;
	}

protected:
	SDL_Window* window;
};
#endif