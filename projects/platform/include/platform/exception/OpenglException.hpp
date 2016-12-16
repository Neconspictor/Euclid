#pragma once
#include <stdexcept>

/**
* This exception should be thrown if anything opengl related couldn't be achieved without any recover possibility. 
*/
class OpenglException : public std::runtime_error
{
public:
	explicit OpenglException(const std::string& _Message)
		: runtime_error(_Message)
	{
	}

	explicit OpenglException(const char* _Message)
		: runtime_error(_Message)
	{
	}
};