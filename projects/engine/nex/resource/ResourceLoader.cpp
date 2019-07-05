#include <nex/resource/ResourceLoader.hpp>
#include <nex/SubSystemProvider.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/resource/Resource.hpp>

std::unique_ptr<nex::ResourceLoader> nex::ResourceLoader::mInstance;

nex::ResourceLoader::ResourceLoader(Window* shared) : mWindow(shared)
{
	mLogger.setPrefix("ResourceLoader");

	mWorker = std::thread([=]()
	{
		run(mWindow);
	});
}

nex::ResourceLoader::~ResourceLoader()
{
	shutdown();
}

void nex::ResourceLoader::init(Window* shared)
{
	mInstance = std::make_unique<ResourceLoader>(shared);
}

nex::ResourceLoader* nex::ResourceLoader::get()
{
	return mInstance.get();
}

const nex::ConcurrentQueue<nex::Resource*>& nex::ResourceLoader::getFinalizeQueue() const
{
	return mFinalizeResources;
}

nex::ConcurrentQueue<nex::Resource*>& nex::ResourceLoader::getFinalizeQueue()
{
	return mFinalizeResources;
}

unsigned long nex::ResourceLoader::getFinishedJobs() const
{
	return mFinishedJobs;
}

unsigned long nex::ResourceLoader::getRequestedJobs() const
{
	return mRequestedJobs;
}

void nex::ResourceLoader::waitTillAllJobsFinished()
{
	std::unique_lock<std::mutex>lock(mMutex);
	mCondition.wait(lock, [=] {return mRequestedJobs == mFinishedJobs; });
}

void nex::ResourceLoader::resetJobCounter()
{
	std::unique_lock<std::mutex>lock(mMutex);
	mFinishedJobs = 0;
	mRequestedJobs = 0;
}

void nex::ResourceLoader::shutdown()
{
	{
		std::unique_lock<std::mutex> lock(mMutex);

		if (!mIsRunning) return;
		mIsRunning = false;
	}

	mCondition.notify_all();

	mWorker.join();
}

void nex::ResourceLoader::run(Window* window)
{
	LOG(mLogger, Info) << "Started";

	
	window->activate(true);
	window->activate();
	auto* backend = RenderBackend::get();
	backend->setViewPort(0, 0, 800, 600);
	backend->setMSAASamples(1);
	backend->init();

	while (true)
	{
		Job t;

		{
			std::unique_lock<std::mutex> lock(mMutex);

			// Wait till there is a job to process or shutdown has been called
			mCondition.wait(lock, [=] { return !mJobs.empty() || !mIsRunning; });

			if (!mIsRunning && mJobs.empty()) return;

			t = std::move(mJobs.front());
			mJobs.pop();
		}
		
		t();
		backend->flushPendingCommands();
	}
}