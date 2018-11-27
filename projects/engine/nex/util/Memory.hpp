#pragma once

namespace nex {

	template<typename T>
	class Guard
	{
	public:

		Guard() : mItem(nullptr) {}

		explicit Guard(T* item)
			: mItem(item)
		{ }

		Guard(const Guard&) = delete;
		Guard& operator=(const Guard&) = delete;
		
		Guard(Guard&& o) noexcept : mItem(o.mItem)
		{
			o.mItem = nullptr;
		}
		Guard& operator=(Guard&& o) = delete;

		T* get() const
		{
			return mItem;
		}

		Guard& operator=(T* item) noexcept
		{
			mItem = item;
			return *this;
		}

		T& operator *() const
		{
			return *mItem;
		}

		T* operator ->() const
		{
			return mItem;
		}

		T& operator [](size_t index) const
		{
			return mItem[index];
		}

		void setContent(T* item)
		{
			mItem = item;
		}

		~Guard()
		{
			delete mItem;
			mItem = nullptr;
		}
	private:
		T* mItem;
	};


	template<typename T>
	class GuardArray
	{
	public:

		GuardArray() : mArray(nullptr){}

		explicit GuardArray(T* arr)
			: mArray(arr)
		{ }

		GuardArray(const GuardArray&) = delete;
		GuardArray& operator=(const GuardArray&) = delete;

		GuardArray(GuardArray&& o) noexcept : mArray(o.mArray)
		{
			o.mArray = nullptr;
		}
		GuardArray& operator=(GuardArray&& o) = delete;

		GuardArray& operator=(T* arr) noexcept
		{
			mArray = arr;
			return *this;
		}

		T* get() const
		{
			return mArray;
		}

		T& operator *() const
		{
			return *mArray;
		}

		T* operator ->() const
		{
			return mArray;
		}

		T& operator [](size_t index) const
		{
			return mArray[index];
		}

		void setContent(T* arr)
		{
			mArray = arr;
		}

		~GuardArray()
		{
			delete[] mArray;
			mArray = nullptr;
		}
	private:
		T* mArray;
	};

	typedef GuardArray<char> MemGuard;
}