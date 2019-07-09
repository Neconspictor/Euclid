#pragma once
#include <mutex>
#include <queue>
#include <condition_variable>
#include "nex/common/Log.hpp"
#include <nex/common/Future.hpp>
#include <nex/common/ConcurrentQueue.hpp>


namespace nex
{
	class SubSystemProvider;
	class Window;
	class Resource;

	class ResourceLoader
	{
	public:
		using Job = std::function<void()>;

		ResourceLoader( Window* shared);

		~ResourceLoader();

		static void init(Window* shared);
		static ResourceLoader* get();


		template <class Func, class... Args
			,class = std::enable_if_t<!std::is_base_of_v<nex::Resource*, std::invoke_result<Func, Args...>>>
		>
			//decltype(std::declval<Func>()(std::declval<Args>()...))
		auto enqueue(Func&& func, Args&&... args) -> Future<nex::Resource*>
		{
			//using Type = decltype(std::declval<Func>()(std::declval<Args>()...));
			//static_assert(Type == nex::Resource*, "Wrong Signature!");

			//decltype(std::declval<Func>()(std::declval<Args>()...))
			//

			auto wrapper = std::make_shared<PackagedTask<nex::Resource*()>>(
				std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
				);

			{
				std::unique_lock<std::mutex>lock(mMutex);
				++mRequestedJobs;

				if (!mIsRunning) throw std::runtime_error("nex::ResourceLoader::enqueue: Already shutdown!");
				mJobs.push(std::move(createJob(wrapper)));
			}

			mCondition.notify_all();

			return wrapper->get_future();
		}

		const nex::ConcurrentQueue<nex::Resource*>& getFinalizeQueue() const;
		nex::ConcurrentQueue<nex::Resource*>& getFinalizeQueue();

		const nex::ConcurrentQueue<std::shared_ptr<std::exception>>& getExceptionQueue() const;
		nex::ConcurrentQueue<std::shared_ptr<std::exception>>& getExceptionQueue();

		
		unsigned long getFinishedJobs() const;
		unsigned long getRequestedJobs() const;
	
		void resetJobCounter();

		void shutdown();

		void waitTillAllJobsFinished();

	private:
		//tbb::concurrent_bounded_queue<Task> mTasks;
		//std::atomic<bool> mIsRunning;
		std::condition_variable mCondition;
		unsigned long mFinishedJobs;
		unsigned long mRequestedJobs;
		std::mutex mMutex;
		bool mIsRunning = true;
		std::queue<Job> mJobs;
		std::thread mWorker;
		nex::Logger mLogger;

		static std::unique_ptr<ResourceLoader> mInstance;
		Window* mWindow;
		nex::ConcurrentQueue<nex::Resource*> mFinalizeResources;
		nex::ConcurrentQueue<std::shared_ptr<std::exception>> mExceptions;

		Job createJob(std::shared_ptr<PackagedTask<nex::Resource*()>> task);


		void run(Window* window);
	};
}