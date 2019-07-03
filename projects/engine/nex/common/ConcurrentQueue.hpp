#pragma once
#include <queue>
#include <mutex>

namespace nex
{
	template <class T>
	class ConcurrentQueue
	{
	public:

		void clear() {
			std::unique_lock<std::mutex> lock(mMutex);
			mQueue.clear();
		}

		bool empty() const
		{
			std::unique_lock<std::mutex> lock(mMutex);
			return mQueue.empty();
		}

		T pop()
		{
			std::unique_lock<std::mutex> lock(mMutex);
			T t = std::move(mQueue.front());
			mQueue.pop();
			return t;
		}

		void push(T t)
		{
			std::unique_lock<std::mutex> lock(mMutex);
			mQueue.push(t);
		}

		size_t size() const
		{
			std::unique_lock<std::mutex> lock(mMutex);
			return mQueue.size();
		}

	private:
		
		mutable std::mutex mMutex;
		std::queue<T> mQueue;
	};
}