#include <NeXEngine.hpp>
#include <csignal>
#include <boost/stacktrace.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/exception/get_error_info.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/opengl/window_system/glfw/SubSystemProviderGLFW.hpp>
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


	nex::SubSystemProviderGLFW* provider = nex::SubSystemProviderGLFW::get();

	std::ofstream logFile("extLog.txt");

	nex::LogSink::get()->registerStream(&std::cout);
	nex::LogSink::get()->registerStream(&logFile);

	nex::LoggerManager* logManager = nex::LoggerManager::get();
	logManager->setMinLogLevel(nex::Debug);


	nex::Logger logger("Main");

	//return EXIT_SUCCESS;

	logLastCrashReport(logger);


	//nex::CullEnvironmentLightsCsCpuShader::test0();
	//return EXIT_SUCCESS;

	//bool isCopyable = std::is_trivially_copyable<nex::Bone>::value;

	//return EXIT_SUCCESS;


	try {

		if (!provider->init())
		{
			nex::throw_with_trace(std::runtime_error("Couldn't initialize window system!"));
		}

		{
			nex::NeXEngine neXEngine(provider);
			neXEngine.init();


			//nex::AnimationManager::init("F:/Development/Repositories/Nec/_work/data/_compiled/anims/", ".CANI", ".CRIG");

/*			auto importScene = nex::ImportScene::read("F:/Development/Repositories/Nec/_work/data/meshes/bob/boblampclean.md5mesh");
			auto* rig = nex::AnimationManager::get()->load(importScene);

			auto importScene2 = nex::ImportScene::read("F:/Development/Repositories/Nec/_work/data/meshes/bob/boblampclean.md5anim");
			auto* rig2 = nex::AnimationManager::get()->load(importScene2);

			nex::BoneAnimationLoader animLoader;
			auto anims = animLoader.load(importScene2.getAssimpScene(), rig2);


			nex::Rig rig3 = nex::Rig::createUninitialized();
			nex::BoneAnimation ani = nex::BoneAnimation::createUnintialized();

			{
				nex::BinStream file(0);
				file.open("bob.CANI", std::ios::out | std::ios::trunc);
				file << anims[0];
			}

			{
				nex::BinStream file(0);
				file.open("bob.CANI", std::ios::in);
				file >> ani;
			}*/

			neXEngine.initScene();
			neXEngine.run();
		}

		LOG(logger, nex::Info) << "Done.";

	} catch (const std::exception& e)
	{
		nex::ExceptionHandling::logExceptionWithStackTrace(logger, e);
	} catch(...)
	{
		LOG(logger, nex::Fault) << "Unknown Exception occurred.";
	}

	nex::MeshManager::release();
	nex::TextureManager::get()->release();
	nex::RenderBackend::get()->release();

	provider->terminate();

	return EXIT_SUCCESS;
}