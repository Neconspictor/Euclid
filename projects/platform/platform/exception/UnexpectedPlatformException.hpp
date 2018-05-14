#pragma once
#include <stdexcept>

/**
* This exception should be thrown if a platform object is retrieved that doesn't match the expected one.
*/
class UnexpectedPlatformException : public std::runtime_error
{
public:
	explicit UnexpectedPlatformException(const std::string& _Message)
		: runtime_error(_Message)
	{
	}

	explicit UnexpectedPlatformException(const char* _Message)
		: runtime_error(_Message)
	{
	}
};