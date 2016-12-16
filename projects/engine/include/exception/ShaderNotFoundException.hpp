#pragma once
#include <stdexcept>

/**
 * This exception is intended to be thrown if a shader 
 * wasn't found.
 */
class ShaderNotFoundException : public std::runtime_error
{
public:
	explicit ShaderNotFoundException(const std::string& _Message)
		: runtime_error(_Message)
	{
	}

	explicit ShaderNotFoundException(const char* _Message)
		: runtime_error(_Message)
	{
	}
};