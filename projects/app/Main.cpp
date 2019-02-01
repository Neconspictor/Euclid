#include <NeXEngine.hpp>
#include <csignal>
#include <boost/stacktrace.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/exception/get_error_info.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include "nex/opengl/window_system/glfw/SubSystemProviderGLFW.hpp"
#include "nex/opengl/model/ModelManagerGL.hpp"


#ifdef WIN32
#include <nex/platform/windows/WindowsPlatform.hpp>
#endif


static const char* CRASH_REPORT_FILENAME = "./crashReport.dump";

void signal_handler(int signum) {
	::signal(signum, SIG_DFL);
	boost::stacktrace::safe_dump_to(CRASH_REPORT_FILENAME);
	std::cout << "Called my_signal_handler" << std::endl;
	::raise(SIGABRT);
}

void logLastCrashReport(nex::Logger& logger)
{
	if (boost::filesystem::exists(CRASH_REPORT_FILENAME)) {
		// there is a backtrace
		std::ifstream ifs(CRASH_REPORT_FILENAME);

		boost::stacktrace::stacktrace st = boost::stacktrace::stacktrace::from_dump(ifs);
		LOG(logger, nex::Fault) << "Previous run crashed:\n" << st;

		// cleaning up
		ifs.close();
		boost::filesystem::remove(CRASH_REPORT_FILENAME);
	}
}


int main(int argc, char** argv)
{
#ifdef WIN32
	nex::initWin32CrashHandler();
#else
	::signal(SIGSEGV, signal_handler);
	::signal(SIGABRT, signal_handler);
#endif

	nex::SubSystemProviderGLFW* provider = nex::SubSystemProviderGLFW::get();

	std::ofstream logFile("extLog.txt");

	nex::LogSink::get()->registerStream(&std::cout);
	nex::LogSink::get()->registerStream(&logFile);

	nex::LoggerManager* logManager = nex::LoggerManager::get();
	logManager->setMinLogLevel(nex::Debug);

	nex::Logger logger("Main");



	logLastCrashReport(logger);

	try {
		if (!provider->init())
		{
			nex::throw_with_trace(std::runtime_error("Couldn't initialize window system!"));
		}

		{
			nex::NeXEngine neXEngine(provider);
			neXEngine.init();
			neXEngine.run();
		}

		LOG(logger, nex::Info) << "Done.";

	} catch (const std::exception& e)
	{
		LOG(logger, nex::Fault) << "Exception: " << typeid(e).name() << ": "<< e.what();
		//LOG(logger, nex::Fault) << "Stack trace: " << boost::stacktrace::stacktrace();
		const boost::stacktrace::stacktrace* st = boost::get_error_info<nex::traced>(e);
		if (st) {
			LOG(logger, nex::Fault) << "Stack trace: " << *st;
		}
	} catch(...)
	{
		LOG(logger, nex::Fault) << "Unknown Exception occurred.";
	}



	nex::StaticMeshManager::get()->release();
	nex::TextureManagerGL::get()->release();

	provider->terminate();

	return EXIT_SUCCESS;
}