#pragma once
#include <stdexcept>

namespace nex
{
	/**
	* This exception is intended to be thrown if a unresolvable error related with shaders occurs (e.g. compiling, linking).
	*/
	class ShaderException : public std::runtime_error
	{
	public:
		explicit ShaderException(const std::string& _Message)
			: runtime_error(_Message)
		{
		}

		explicit ShaderException(const char* _Message)
			: runtime_error(_Message)
		{
		}
	};
}