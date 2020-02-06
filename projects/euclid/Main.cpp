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
#include <nex/cluster/Cluster.hpp>
#include <nex/import/ImportScene.hpp>
#include <nex/anim/AnimationManager.hpp>
#include <nex/anim/BoneAnimationLoader.hpp>
#include <nex/mesh/MeshLoader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
//#include <Magick++.h>
//#include <MagickCore/quantum.h>
#include <list>

#ifdef WIN32
#include <nex/platform/windows/WindowsPlatform.hpp>
#endif


static const char* CRASH_REPORT_FILENAME = "./crashReport.dump";

/*void signal_handler(int signum) {
	::signal(signum, SIG_DFL);
	boost::stacktrace::safe_dump_to(CRASH_REPORT_FILENAME);
	std::cout << "Called my_signal_handler" << std::endl;
	::raise(SIGABRT);
}*/

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


#include <eh.h>


//#pragma unmanaged
void my_trans_func(unsigned int u, PEXCEPTION_POINTERS)
{
	//throw std::exception("test!");
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


int main(int argc, char** argv)
{

	



#ifdef WIN32
	//nex::initWin32CrashHandler();
#else
	::signal(SIGSEGV, signal_handler);
	::signal(SIGABRT, signal_handler);
#endif


	// Be sure to enable "Yes with SEH Exceptions (/EHa)" in C++ / Code Generation;
	_set_se_translator(my_trans_func);

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



	auto rot = rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1, 0, 1));
	auto scaleM = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f,1.0f, 1.3f));
	auto trafo = scaleM * rot;

	glm::vec3 pos, scale, skew;
	glm::quat rotation;
	glm::vec4 persp;
	glm::decompose(trafo, scale, rotation, pos, skew, persp);

	auto shearXY = glm::mat4(1.0f); shearXY[1][0] = skew.z;
	auto shearYZ = glm::mat4(1.0f); shearYZ[2][1] = skew.x;
	auto shearXZ = glm::mat4(1.0f); shearXZ[2][0] = skew.y;
	auto shearMatrix = shearYZ * shearXZ * shearXY;

	const auto temp = glm::mat4();
	const auto rotationMatC = toMat4(rotation);
	const auto scaleMatC = glm::scale(temp, scale);
	const auto transMatC = translate(temp, pos);

	auto composed = transMatC * rotationMatC * shearMatrix * scaleMatC;



	


	//nex::CullEnvironmentLightsCsCpuShader::test0();
	//return EXIT_SUCCESS;

	//bool isCopyable = std::is_trivially_copyable<nex::Bone>::value;

	//return EXIT_SUCCESS;


	nex::SubSystemProviderGLFW* provider = nex::SubSystemProviderGLFW::get();

	try {

		//Magick::InitializeMagick(*argv);
		//std::list<Magick::Image> imageList;
		//readImages(&imageList, "C:/Development/Repositories/Euclid/_work/data/textures/nature/tileable_wood_texture.jpg");
		

		//Magick::TerminateMagick();

		if (true) {
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