#ifndef SDL_INIT_EXCEPTION_HPP
#define SDL_INIT_EXCEPTION_HPP

#include <exception>
#include <string>
#include <SDL/SDL.h>

class InitError : public std::exception {
public:
	InitError();
	InitError(const std::string&);
	virtual ~InitError() throw();
	virtual const char* what() const throw() override;
private:
	std::string msg;
};

inline InitError::InitError() :
	exception(), msg(SDL_GetError()) {}

inline InitError::InitError(const std::string& m) :
	exception(), msg(m) {}

inline InitError::~InitError() throw() {}

inline const char* InitError::what() const throw() {
	return msg.c_str();
}

#endif