#pragma once
#include <stdexcept>

namespace nex
{
	/**
	* This exception should be thrown if a resource couldn't be loaded.
	*/
	class ResourceLoadException : public std::runtime_error
	{
	public:
		explicit ResourceLoadException(const std::string& _Message)
			: runtime_error(_Message)
		{
		}

		explicit ResourceLoadException(const char* _Message)
			: runtime_error(_Message)
		{
		}
	};
}