#include <NeXEngine.hpp>
#include <nex/logging/GlobalLoggingServer.hpp>
#include <csignal>
#include <boost/stacktrace.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/exception/get_error_info.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include "nex/opengl/window_system/glfw/SubSystemProviderGLFW.hpp"

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

void logLastCrashReport(nex::LoggingClient& logger)
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
	initWin32CrashHandler();
#else
	::signal(SIGSEGV, signal_handler);
	::signal(SIGABRT, signal_handler);
#endif

	nex::LoggingClient logger(nex::getLogServer());
	SubSystemProviderGLFW* provider = SubSystemProviderGLFW::get();


	logLastCrashReport(logger);

	ext::Logger extLogger;
	extLogger.setPrefix("extLogger");
	extLogger.setMinLogLevel(ext::Debug);
	extLogger(__FILE__, __FUNCTION__, __LINE__, ext::Info) << "A cool message!";

	try {
		if (!provider->init())
		{
			//LOG(m_logClient, platform::Fault) << "Couldn't initialize window system!";
			nex::getLogServer()->terminate();
			throw_with_trace(std::runtime_error("Couldn't initialize window system!"));
		}

		NeXEngine neXEngine(provider);
		neXEngine.init();
		neXEngine.run();
		LOG(logger, nex::Info) << "Done.";

	} catch (const std::exception& e)
	{
		LOG(logger, nex::Fault) << "Exception: " << typeid(e).name() << ": "<< e.what();
		//LOG(logger, nex::Fault) << "Stack trace: " << boost::stacktrace::stacktrace();
		const boost::stacktrace::stacktrace* st = boost::get_error_info<traced>(e);
		if (st) {
			LOG(logger, nex::Fault) << "Stack trace: " << *st;
		}
	} catch(...)
	{
		LOG(logger, nex::Fault) << "Unknown Exception occurred.";
	}



	ModelManagerGL::get()->release();
	TextureManagerGL::get()->release();

	provider->terminate();

	nex::shutdownLogServer();

	return EXIT_SUCCESS;
}