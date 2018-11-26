#pragma once

namespace nex {

	template<typename T>
	class MemoryGuard
	{
	public:

		MemoryGuard() : mItem(nullptr) {}

		explicit MemoryGuard(T* item)
			: mItem(item)
		{ }

		MemoryGuard(const MemoryGuard&) = delete;
		MemoryGuard& operator=(const MemoryGuard&) = delete;
		
		MemoryGuard(MemoryGuard&& o) noexcept : mItem(o.mItem)
		{
			o.mItem = nullptr;
		}
		MemoryGuard& operator=(MemoryGuard&& o) noexcept
		{
			if (this == &o) return *this;
			mItem = o.mItem;
			o.mItem = nullptr;

			return *this;
		}

		T* get() const
		{
			return mItem;
		}

		T* operator *() const
		{
			return mItem;
		}

		void setContent(T* item)
		{
			mItem = item;
		}

		~MemoryGuard()
		{
			delete mItem;
			mItem = nullptr;
		}
	private:
		T* mItem;
	};


	template<typename T>
	class MemoryGuardArray
	{
	public:

		MemoryGuardArray() : mArray(nullptr){}

		explicit MemoryGuardArray(T* arr)
			: mArray(arr)
		{ }

		MemoryGuardArray(const MemoryGuardArray&) = delete;
		MemoryGuardArray& operator=(const MemoryGuardArray&) = delete;

		MemoryGuardArray(MemoryGuardArray&& o) noexcept : mArray(o.mArray)
		{
			o.mArray = nullptr;
		}
		MemoryGuardArray& operator=(MemoryGuardArray&& o) noexcept
		{
			if (this == &o) return *this;
			mArray = o.mArray;
			o.mArray = nullptr;

			return *this;
		}

		MemoryGuardArray& operator=(T* arr) noexcept
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

		~MemoryGuardArray()
		{
			delete[] mArray;
			mArray = nullptr;
		}
	private:
		T* mArray;
	};

	typedef MemoryGuardArray<char> MemoryWrapper;
}