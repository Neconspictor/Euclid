#include <NeXEngine.hpp>
#include <csignal>
#include <boost/stacktrace.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/exception/get_error_info.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/opengl/window_system/glfw/SubSystemProviderGLFW.hpp>
#include <nex/mesh/StaticMeshManager.hpp>
#include <nex/texture/TextureManager.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include "nex/common/File.hpp"
#include "nex/mesh/MeshStore.hpp"
#include "nex/math/Circle.hpp"
#include "nex/math/Sphere.hpp"


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

	//nex::MeshStore::test();

	//nex::MaterialStore::test();


	nex::Logger logger("Main");

	nex::Plane plane(glm::vec3(0, 0, 1), glm::vec3(0.0f, 0.0f, 0.0f));
	nex::Circle3D circle(std::move(plane), glm::vec3(0.0, 0.0, 0.0), 1.0f);
	nex::Ray ray(glm::vec3(1.0f, 0.0, 0.0), glm::vec3(1.0f, 1.0f, 0.01f));
	nex::Sphere sphere = {glm::vec3(0.0f, 0.0f, 0.0f), 1.0f};
	const auto result = ray.intersects(circle);
	//const auto result = ray.intersects(sphere);

	std::cout << "result.intersectionCount = " << result.intersectionCount << std::endl;
	std::cout << "result.firstMultiplier = " << result.firstMultiplier 
			<< ", first intersection = " << ray.getPoint(result.firstMultiplier)
			<< ", length = " << length(ray.getPoint(result.firstMultiplier)) <<  std::endl;
	std::cout << "result.secondMultiplier = " << result.secondMultiplier 
				<< ", second intersection = " << ray.getPoint(result.secondMultiplier) 
				<< ", length = " << length(ray.getPoint(result.secondMultiplier)) << std::endl;

	//glm::vec3 point(2.0f, 89.0f, 67.0f);
	//const auto projectedPoint = plane.project(point);
	//std::cout << "point = " << point << std::endl;
	//std::cout << "projected point = " << projectedPoint << std::endl;



	//return EXIT_SUCCESS;

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
	nex::TextureManager::get()->release();
	nex::RenderBackend::get()->release();

	provider->terminate();

	return EXIT_SUCCESS;
}