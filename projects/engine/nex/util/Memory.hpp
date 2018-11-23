#pragma once

namespace nex::util {

	class MemoryWrapper
	{
	public:
		explicit MemoryWrapper(char* value)
			: _value(value)
		{ }

		MemoryWrapper(const MemoryWrapper&) = delete;
		MemoryWrapper(MemoryWrapper&&) = delete;

		MemoryWrapper& operator=(const MemoryWrapper&) = delete;
		MemoryWrapper& operator=(MemoryWrapper&&) = delete;

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
}