#include <Euclid.hpp>
#include <csignal>
#include <boost/stacktrace.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/exception/get_error_info.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/platform/glfw/SubSystemProviderGLFW.hpp>
#include <nex/mesh/MeshManager.hpp>
#include <nex/texture/TextureManager.hpp>
#include "nex/renderer/RenderBackend.hpp"
#include "nex/common/File.hpp"
#include "nex/mesh/MeshStore.hpp"
#include "nex/math/Circle.hpp"
#include "nex/math/Sphere.hpp"
#include <nex/common/Future.hpp>
#include <nex/pbr/Cluster.hpp>
#include <nex/import/ImportScene.hpp>
#include <nex/anim/AnimationManager.hpp>
#include <nex/anim/BoneAnimationLoader.hpp>
#include <nex/mesh/MeshLoader.hpp>
#include <glm/gtc/matrix_transform.hpp>


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

	//nex::FutureTest();
	//return EXIT_SUCCESS;

	std::ofstream logFile("extLog.txt");

	nex::LogSink::get()->registerStream(&std::cout);
	nex::LogSink::get()->registerStream(&logFile);

	nex::LoggerManager* logManager = nex::LoggerManager::get();
	logManager->setMinLogLevel(nex::Debug);


	nex::Logger logger("Main");

	//return EXIT_SUCCESS;

	logLastCrashReport(logger);

	std::filesystem::path animFolder = "F:/Development/Repositories/Euclid/_work/data/_compiled/anims";
	std::filesystem::remove_all(animFolder);



	auto localTrans = translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	auto localRot = rotate(glm::mat4(1.0f), glm::radians(72.0f), glm::vec3(1, 1, 0));
	auto rot = rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1, 0, 1));
	auto trans = translate(glm::mat4(1.0f), glm::vec3(0, 3.0f, 0.0f));
	auto scaleParent = scale(glm::mat4(1.0f), glm::vec3(0.1f));
	auto toOrigin = translate(glm::mat4(1.0f), -glm::vec3(trans[3]));
	auto fromOrigin = translate(glm::mat4(1.0f), glm::vec3(trans[3]));
	auto parentTrafo =  
		trans
		* rot
		* scaleParent;
	auto scaleTrafo = fromOrigin * scale(glm::mat4(1.0f), glm::vec3(1, 2, 1)) * toOrigin;
	auto scaleTrafo2 =
		inverse(parentTrafo)
		* scaleTrafo
		* parentTrafo;

	glm::mat4 mat = scaleTrafo * parentTrafo * localTrans * localRot;

	glm::mat4 mat2 = parentTrafo * scaleTrafo2 * localTrans * localRot;
	



	//nex::CullEnvironmentLightsCsCpuShader::test0();
	//return EXIT_SUCCESS;

	//bool isCopyable = std::is_trivially_copyable<nex::Bone>::value;

	//return EXIT_SUCCESS;


	nex::SubSystemProviderGLFW* provider = nex::SubSystemProviderGLFW::get();

	try {

		if (!provider->init())
		{
			nex::throw_with_trace(std::runtime_error("Couldn't initialize window system!"));
		}

		{
			nex::Euclid euclid(provider);
			euclid.init();
			euclid.initScene();
			euclid.run();
		}

		LOG(logger, nex::Info) << "Done.";

	} catch (const std::exception& e)
	{
		nex::ExceptionHandling::logExceptionWithStackTrace(logger, e);
	} catch(...)
	{
		LOG(logger, nex::Fault) << "Unknown Exception occurred.";
	}

	nex::gui::ImGUI_Impl::get()->release();
	nex::MeshManager::release();
	nex::TextureManager::get()->release();
	nex::RenderBackend::get()->release();

	provider->terminate();

	return EXIT_SUCCESS;
}