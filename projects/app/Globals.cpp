#include <Globals.hpp>
#include <nex/config/Configuration.hpp>

namespace nex::util {

	void Globals::init(Configuration* globalConfig)
	{
		mRoot = canonical(std::filesystem::path("./")).generic_string();
		if (globalConfig == nullptr) return;

		std::string option = globalConfig->get<std::string>(CONFIGURATION_ROOT_DIRECTORY_KEY);

		if (option == "") return;

		std::filesystem::path root(option);

		if (std::filesystem::exists(root) && std::filesystem::is_directory(root))
		{
			std::filesystem::path canonicalPath = canonical(root);

			// we add a trailing slash so that it is later easier to construct subdirectories
			mRoot = canonicalPath.generic_string() + "/";
		}
	}

	std::string Globals::getCompiledMeshFolder()
	{
		return getCompiledRootFolder() + "meshes/";
	}

	const std::string& Globals::getCompiledMeshFileExtension()
	{
		static const std::string extension = ".CMESH";
		return extension;
	}

	std::string Globals::getCompiledPbrFolder()
	{
		return getCompiledRootFolder() + "probes/";
	}

	const std::string& Globals::getCompiledTextureFileExtension()
	{
		static const std::string extension = ".CTEX";
		return extension;
	}


	std::string Globals::getCompiledRootFolder()
	{
		/**
		 * Root folder for compilations
		 */
		static const std::string COMPILED_SUBFOLDER = "_work/data/_compiled/";

		return getRootDirectory() + COMPILED_SUBFOLDER;
	}

	std::string Globals::getMeshesPath()
	{
		return getRootDirectory() + "_work/data/meshes/";
	}

	std::string Globals::getOpenGLShaderPath()
	{
		return getRootDirectory() + "shaders/opengl/";
	}

	std::string Globals::getTexturePath()
	{
		return getRootDirectory() + "_work/data/textures/";
	}

	const std::string& Globals::getRootDirectory()
	{
		return mRoot;
	}
}