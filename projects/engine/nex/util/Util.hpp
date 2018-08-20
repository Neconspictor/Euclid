#pragma once

#include <nex/util/StringUtils.hpp>
#include <nex/util/PointerUtils.hpp>
#include <nex/util/TimeUtils.hpp>
#include <nex/util/MathUtils.hpp>

struct Dimension
{
	int xPos;
	int yPos;
	int width;
	int height;
};

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