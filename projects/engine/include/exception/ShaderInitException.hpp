#ifndef ENGINE_EXCEPTION_SHADER_INIT_FOUND_EXCEPTION_HPP
#define ENGINE_EXCEPTION_SHADER_INIT_FOUND_EXCEPTION_HPP
#include <stdexcept>

/**
* This exception is intended to be thrown if a shader
* should be loaded but couldn't successfully be initialized.
*/
class ShaderInitException : public std::runtime_error
{
public:
	explicit ShaderInitException(const std::string& _Message)
		: runtime_error(_Message)
	{
	}

	explicit ShaderInitException(const char* _Message)
		: runtime_error(_Message)
	{
	}
};

#endif