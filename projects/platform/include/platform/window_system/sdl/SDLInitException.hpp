#pragma once

#include <exception>
#include <string>
#include <SDL/SDL.h>

/**
 * An exception class that represents an exception while initiliazing a SDL context.
 */
class SDLInitError : public std::exception {
public:
	SDLInitError();
	SDLInitError(const std::string&);
	virtual ~SDLInitError() throw();
	virtual const char* what() const throw() override;
private:
	std::string msg;
};

inline SDLInitError::SDLInitError() :
	exception(), msg(SDL_GetError()) {}

/**
 * Creates a new sdl initialization exception and puts a description, what did go wrong, to it.
 */
inline SDLInitError::SDLInitError(const std::string& m) :
	exception(), msg(m) {}

inline SDLInitError::~SDLInitError() throw() {}

/**
 * Provides the exception descrption.
 */
inline const char* SDLInitError::what() const throw() {
	return msg.c_str();
}