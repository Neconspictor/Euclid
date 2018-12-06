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

		T* get()
		{
			return mItem;
		}

		T* get() const
		{
			return mItem;
		}

		Guard& operator=(T* item) noexcept
		{
			this->~Guard();
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
			this->~Guard();
			mItem = item;
		}

		~Guard()
		{
			delete mItem;
			mItem = nullptr;
		}

		T* reset()
		{
			T* backup = mItem;
			mItem = nullptr;
			return backup;
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
			this->~GuardArray();
			mArray = arr;
			return *this;
		}

		T* get()
		{
			return mArray;
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

		T* reset()
		{
			T* backup = mArray;
			mArray = nullptr;
			return backup;
		}

		void setContent(T* arr)
		{
			this->~GuardArray();
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
