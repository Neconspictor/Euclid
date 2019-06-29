#pragma once
#include <mutex>
#include <queue>
#include <condition_variable>
#include <future>
#include "nex/common/Log.hpp"

namespace nex
{
	class SubSystemProvider;
	class Window;

	class ResourceLoader
	{
	public:
		using Job = std::function<void()>;

		ResourceLoader( Window* shared);

		~ResourceLoader();

		static void init(Window* shared);
		static ResourceLoader* get();



		template <class Func, class... Args>
		auto enqueue(Func&& func, Args&&... args)-> std::future<decltype(std::declval<Func>()(std::declval<Args>()...))>
		{

			using Type = decltype(std::declval<Func>()(std::declval<Args>()...));
			auto wrapper = std::make_shared<std::packaged_task<Type()>>(
				std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
				);

			{
				std::unique_lock<std::mutex>lock(mMutex);
				if (!mIsRunning) throw std::runtime_error("nex::ResourceLoader::enqueue: Already shutdown!");
				mJobs.push([=]
				{
					(*wrapper)();
				});
			}

			mCondition.notify_one();

			return wrapper->get_future();
		}

		void shutdown();

	private:
		//tbb::concurrent_bounded_queue<Task> mTasks;
		//std::atomic<bool> mIsRunning;
		std::condition_variable mCondition;
		std::mutex mMutex;
		bool mIsRunning = true;
		std::queue<Job> mJobs;
		std::thread mWorker;
		nex::Logger mLogger;

		static std::unique_ptr<ResourceLoader> mInstance;
		Window* mWindow;


		void run(Window* window);
	};
}