#include <Globals.hpp>
#include <nex/config/Configuration.hpp>

namespace nex::util {

	std::string Globals::rootDirectory = "./";

	void Globals::initGlobals()
	{
		rootDirectory = canonical(std::filesystem::path("./")).generic_string();
		Configuration* globalConfig = Configuration::getGlobalConfiguration();
		
		if (globalConfig == nullptr) return;

		std::string option = globalConfig->get<std::string>("General.rootDirectory");
		
		if (option == "") return;

		std::filesystem::path folder(option);

		if (std::filesystem::exists(folder) && std::filesystem::is_directory(folder))
		{
			std::filesystem::path canonicalPath = canonical(folder);

			// we add a trailing slash so that it is later easier to construct subdirectories
			rootDirectory = canonicalPath.generic_string() + "/"; 
		}
	}

	std::string Globals::getMeshesPath()
	{

		return rootDirectory + MESHES_PATH;
	}

	std::string Globals::getOpenGLShaderPath()
	{
		return rootDirectory + SHADER_PATH_OPENGL;
	}

	std::string Globals::getTexturePath()
	{
		return rootDirectory + TEXTURE_PATH;
	}

	std::string Globals::getRootDirectory()
	{
		return rootDirectory;
	}
}