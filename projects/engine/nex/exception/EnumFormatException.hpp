#pragma once
#include <stdexcept>

namespace nex
{
	/**
	 * This exception should be thrown if an enum couldn't successfully converted e.g. from a string.
	 */
	class EnumFormatException : public std::runtime_error
	{
	public:
		explicit EnumFormatException(const std::string& _Message)
			: runtime_error(_Message)
		{
		}

		explicit EnumFormatException(const char* _Message)
			: runtime_error(_Message)
		{
		}
	};
}