#pragma once
#include <atomic>
#include <mutex>

namespace nex
{
	template<class T> class Future;
	template<class T> class Promise;

	template<class T>
	class _FutureSharedState {
	public:
		T get() {
			std::unique_lock<std::mutex> lock(mMutex);
			mCondition.wait(lock, [=] {return mFinished.load(); });
			return std::move(mElem);
		}

		bool is_ready() const
		{
			return mFinished;
		}

		void set_value(T elem)
		{
			{
				std::unique_lock<std::mutex> lock(mMutex);
				mElem = std::move(elem);
				mFinished = true;
			}

			mCondition.notify_all();
		}

	private:
		friend Future;
		friend Promise;

		T mElem;
		std::condition_variable mCondition;
		std::mutex mMutex;
		std::atomic<bool> mFinished = false;
	};

	template<class T>
	class Future {
	public:

		Future() : mSharedState(std::make_shared<_FutureSharedState<T>>()) 
		{

		}

		T get() 
		{
			return mSharedState->get();
		}

		bool is_ready() const 
		{
			return mSharedState->is_ready();
		}

	private:
		friend Promise;

		std::shared_ptr<_FutureSharedState<T>> mSharedState;
	};

	template<class T>
	class Promise {
	public:

		Promise() : mSharedState(std::make_shared<_FutureSharedState<T>>()) 
		{
		}

		Future<T> get_future()
		{
			Future<T> future;
			future.mSharedState = mSharedState;
			return future;
		}

		void set_value(T elem)
		{
			mSharedState->set_value(std::move(elem));
		}

	private:
		std::shared_ptr<_FutureSharedState<T>> mSharedState;
	};

	static void FutureTest();
}