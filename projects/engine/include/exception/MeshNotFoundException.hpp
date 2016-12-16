#pragma once
#include <stdexcept>

/**
* This exception is intended to be thrown if a mesh
* wasn't found.
*/
class MeshNotFoundException : public std::runtime_error
{
public:
	explicit MeshNotFoundException(const std::string& _Message)
		: runtime_error(_Message)
	{
	}

	explicit MeshNotFoundException(const char* _Message)
		: runtime_error(_Message)
	{
	}
};