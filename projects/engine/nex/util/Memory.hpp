#pragma once

#include <memory>

namespace nex {

	template<class T>
	void setUnique(std::unique_ptr<T>& smart, T&& t) {
		if (!smart) smart = std::make_unique<T>(std::move(t));
		else *smart = std::move(t);
	}


	template<class T>
	struct flexible_ptr {

		explicit flexible_ptr(T* ptr, bool isOwned) noexcept : mPtr(ptr), mIsOwned(isOwned)
		{
		}

		flexible_ptr(flexible_ptr<T>&& o) noexcept : flexible_ptr<T>(nullptr, false) {
			*this = std::forward<flexible_ptr<T>>(o);
		}

		flexible_ptr(std::unique_ptr<T>&& uniquePtr) noexcept  {
			*this = std::forward<std::unique_ptr<T>>(uniquePtr);
		}

		flexible_ptr& operator=(flexible_ptr<T>&& o) noexcept  {

			if (this == &o) return *this;
			std::swap(mPtr, o.mPtr);
			std::swap(mIsOwned, o.mIsOwned);
			return *this;
		}

		flexible_ptr& operator=(nullptr_t) noexcept {
			reset();
			return *this;
		}

		flexible_ptr& operator=(std::unique_ptr<T>&& uniquePtr) noexcept {
			mIsOwned = true;
			mPtr = uniquePtr.release();
			return *this;
		}

		flexible_ptr(const flexible_ptr& o) noexcept = delete;
		flexible_ptr& operator=(const flexible_ptr& o) noexcept = delete;


		~flexible_ptr() {
			if (mIsOwned) delete mPtr;
		}


		bool operator==(const flexible_ptr<T>& o) {
			return o.mPtr == mPtr 
				&& o.mIsOwned == mIsOwned;
		}

		T* operator->() noexcept {
			return get();
		}

		const T* operator->() const noexcept {
			return get();
		}


		T* get() noexcept
		{ 
			return mPtr; 
		}

		const T* get() const noexcept
		{ 
			return mPtr; 
		};
		
		T* release() noexcept {
			mIsOwned = false;
			return mPtr;
		}

		void reset(T* newPtr = nullptr, bool isOwned = true) {
			if (mIsOwned) {
				delete mPtr;
			}
			mPtr = newPtr;
			mIsOwned = isOwned;
		}

	private:
		T* mPtr;
		bool mIsOwned;
	};




	template<class T>
	struct flexible_ptr<T[]> {

		explicit flexible_ptr(T* ptr, bool isOwned) noexcept : mPtr(ptr), mIsOwned(isOwned)
		{
		}

		flexible_ptr(flexible_ptr<T[]>&& o) noexcept : flexible_ptr<T[]>(nullptr, false) {
			*this = std::forward<flexible_ptr<T[]>>(o);
		}

		flexible_ptr(std::unique_ptr<T[]>&& uniquePtr) noexcept {
			*this = std::forward<std::unique_ptr<T[]>>(uniquePtr);
		}

		flexible_ptr& operator=(flexible_ptr<T[]>&& o) noexcept {

			if (this == &o) return *this;
			std::swap(mPtr, o.mPtr);
			std::swap(mIsOwned, o.mIsOwned);
			return *this;
		}

		flexible_ptr& operator=(nullptr_t) noexcept {
			reset();
			return *this;
		}

		flexible_ptr& operator=(std::unique_ptr<T[]>&& uniquePtr) noexcept {
			mIsOwned = true;
			mPtr = uniquePtr.release();
			return *this;
		}

		flexible_ptr(const flexible_ptr& o) noexcept = delete;
		flexible_ptr& operator=(const flexible_ptr& o) noexcept = delete;


		~flexible_ptr() {
			if (mIsOwned) delete[] mPtr;
		}


		bool operator==(const flexible_ptr<T[]>& o) {
			return o.mPtr == mPtr
				&& o.mIsOwned == mIsOwned;
		}

		T* operator->() noexcept {
			return get();
		}

		const T* operator->() const noexcept {
			return get();
		}


		T* get() noexcept
		{
			return mPtr;
		}

		const T* get() const noexcept
		{
			return mPtr;
		};

		T* release() noexcept {
			mIsOwned = false;
			return mPtr;
		}

		void reset(T* newPtr = nullptr, bool isOwned = true) {
			if (mIsOwned) {
				delete[] mPtr;
			}
			mPtr = newPtr;
			mIsOwned = isOwned;
		}

	private:
		T* mPtr;
		bool mIsOwned;
	};
}