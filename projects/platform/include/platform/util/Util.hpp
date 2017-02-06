#pragma once

#include <platform/util/StringUtils.hpp>
#include <platform/util/PointerUtils.hpp>
#include <platform/util/TimeUtils.hpp>


class MemoryWrapper
{
public:
	explicit MemoryWrapper(char* value)
		: _value(value)
	{ }

	char* operator *()
	{
		return _value;
	}

	void setContent(char* content)
	{
		_value = content;
	}

	virtual ~MemoryWrapper()
	{
		if (_value)
		{
			delete[] _value;
			_value = nullptr;
		}
	}
private:
	char* _value;
};