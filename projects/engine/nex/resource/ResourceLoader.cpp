#include <nex/resource/ResourceLoader.hpp>
#include <nex/platform/SubSystemProvider.hpp>
#include <nex/renderer/RenderBackend.hpp>
#include <nex/resource/Resource.hpp>
#include <boost/stacktrace.hpp>
#include <boost/exception/get_error_info.hpp>
#include <boost/filesystem/operations.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/renderer/RenderEngine.hpp>

#include <eh.h>


class SEH_HANDLER {
public:

	static void my_trans_func(unsigned int u, PEXCEPTION_POINTERS)
	{
		std::string error = "SE Exception: ";
		switch (u) {
		case 0xC0000005:
			error += "Access Violation";
			break;
		default:
			char result[11];
			sprintf_s(result, 11, "0x%08X", u);
			error += result;
		};
		nex::throw_with_trace(std::exception(error.c_str()));
	}

};

std::unique_ptr<nex::ResourceLoader> nex::ResourceLoader::mInstance;

nex::ResourceLoader::ResourceLoader(Window* shared, const nex::RenderEngine& renderEngine) : mWindow(shared)
{
	mLogger.setPrefix("ResourceLoader");

	mWorker = std::thread([=]()
	{
			Logger logger("ResourceLoader - Worker");

			_set_se_translator(SEH_HANDLER::my_trans_func);


			try {
				run(mWindow);
			}
			catch (const std::exception & e)
			{
				nex::ExceptionHandling::logExceptionWithStackTrace(logger, e);
			}
			catch (...)
			{
				LOG(logger, nex::Fault) << "Unknown Exception occurred.";
			}
	});
}

nex::ResourceLoader::~ResourceLoader()
{
	shutdownSelf();
}

void nex::ResourceLoader::init(Window* shared, const RenderEngine& renderEngine)
{
	mInstance = std::make_unique<ResourceLoader>(shared, renderEngine);
}

nex::ResourceLoader* nex::ResourceLoader::get()
{
	return mInstance.get();
}

void nex::ResourceLoader::shutdown()
{
	mInstance = nullptr;
}

void nex::ResourceLoader::finalizeAsync(nex::Resource* resource) {
	//get()->enqueue([=]()->nex::Resource* {
	RenderEngine::getCommandQueue()->push([=]() {
		resource->finalize();
	});
	//	return nullptr;
	//});
}

const nex::ConcurrentQueue<std::shared_ptr<std::exception>>& nex::ResourceLoader::getExceptionQueue() const
{
	return mExceptions;
}

nex::ConcurrentQueue<std::shared_ptr<std::exception>>& nex::ResourceLoader::getExceptionQueue()
{
	return mExceptions;
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

void nex::ResourceLoader::shutdownSelf()
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
	backend->init({ 0, 0, 800, 600 }, 1);
	backend->initEffectLibrary();

	while (true)
	{
		Job t;

		{
			std::unique_lock<std::mutex> lock(mMutex);

			// Wait till there is a job to process or shutdown has been called
			mCondition.wait(lock, [=] { return !mJobs.empty() || !mIsRunning; });

			if (!mIsRunning && mJobs.empty()) break;

			t = std::move(mJobs.front());
			mJobs.pop();
		}
		
		t();
		backend->flushPendingCommands();
	}

	backend->release();
}