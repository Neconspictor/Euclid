#ifndef CONCURRENT_QUEUE_HPP
#define CONCURRENT_QUEUE_HPP

/************************************************************************************
*
*	This class is from GrandMaster's tutorial:
*  http://www.grandmaster.nu/blog/?page_id=261
*
************************************************************************************/

#include <queue>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

namespace platform {

	template <typename T>
	class ConcurrentQueue {
	private:
		typedef std::queue<T> Queue;
		typedef boost::mutex Mutex;
		typedef Mutex::scoped_lock ScopedLock;
		typedef boost::condition_variable Condition;

	public:
		bool empty() const;
		size_t size() const;
		void push(const T& value);
		bool try_pop(T& result);
		T wait_pop();

	private:
		Queue mQueue;
		mutable Mutex mMutex;
		Condition mCondition;
	};

	/* implementation */
	template <typename T>
	bool ConcurrentQueue<T>::empty() const {
		ScopedLock lock(mMutex);
		return mQueue.empty();
	}

	template <typename T>
	size_t ConcurrentQueue<T>::size() const {
		ScopedLock lock(mMutex);
		return mQueue.size();
	}

	template <typename T>
	void ConcurrentQueue<T>::push(const T& value) {
		ScopedLock lock(mMutex);

		mQueue.push(value);
		lock.unlock();

		mCondition.notify_one();
	}

	template <typename T>
	bool ConcurrentQueue<T>::try_pop(T& result) {
		ScopedLock lock(mMutex);

		if (mQueue.empty())
			return false;

		result = mQueue.front();
		mQueue.pop();

		return true;
	}

	template <typename T>
	T ConcurrentQueue<T>::wait_pop() {
		ScopedLock lock(mMutex);

		while (mQueue.empty())
			mCondition.wait(lock);

		T result(mQueue.front());
		mQueue.pop();

		return result;
	}
}
#endif