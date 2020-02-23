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

		explicit flexible_ptr(T* ptr, bool isOwned) noexcept : mPtr(ptr), mIsOwned(isOwned),
			mResource(std::make_shared<std::unique_ptr<T>>(mPtr))
		{
		}

		flexible_ptr(nullptr_t) noexcept : mPtr(nullptr), mIsOwned(false),
			mResource(std::make_shared<std::unique_ptr<T>>(nullptr))
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
			std::swap(mResource, o.mResource);
			return *this;
		}

		flexible_ptr& operator=(nullptr_t) noexcept {
			reset();
			return *this;
		}

		flexible_ptr& operator=(std::unique_ptr<T>&& uniquePtr) noexcept {
			mIsOwned = true;
			mResource = std::make_shared<std::unique_ptr<T>>(std::move(uniquePtr));
			mPtr = (*mResource).get();
			return *this;
		}

		flexible_ptr(const flexible_ptr& o) noexcept {
			mIsOwned = o.mIsOwned;
			mResource = o.mResource;
			mPtr = o.mPtr;

		}
		flexible_ptr& operator=(const flexible_ptr& o) noexcept {
			
			if (this == &o) return *this;
			reset();

			mIsOwned = o.mIsOwned;
			mResource = o.mResource;
			mPtr = o.mPtr;

			return *this;
		}


		~flexible_ptr() {
			if (!mIsOwned && mResource.unique()) mResource->release();
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

		operator bool() const {
			return mPtr != nullptr;
		}

		bool operator!() const {
			return mPtr == nullptr;
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
			if (!mResource.unique()) {
				mResource = std::make_shared<std::unique_ptr<T>>(nullptr);
			}
			return mPtr;
		}

		void reset(T* newPtr = nullptr, bool isOwned = true) {
			if (!mIsOwned && mResource.unique()) {
				mResource->release();
			}

			mResource = std::make_shared<std::unique_ptr<T>>(newPtr);
			mPtr = newPtr;
			mIsOwned = isOwned;
		}

	private:
		T* mPtr; // just for faster access
		std::shared_ptr<std::unique_ptr<T>> mResource;
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

		operator bool() const {
			return mPtr != nullptr;
		}

		bool operator!() const {
			return mPtr == nullptr;
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

	template<class T>
	[[nodiscard]] nex::flexible_ptr<T> make_not_owning(T* value) {
		return nex::flexible_ptr<T>(value, false);
	}

	template<class T>
	[[nodiscard]] nex::flexible_ptr<T> make_owning(T* value) {
		return nex::flexible_ptr<T>(value, true);
	}
}