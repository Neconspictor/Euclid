#include <Globals.hpp>
#include <nex/config/Configuration.hpp>
#include <nex/util/ExceptionHandling.hpp>

namespace nex::util {

	void Globals::init(Configuration* globalConfig)
	{
		mRoot = canonical(std::filesystem::path("./")).generic_string() + "/";
		if (globalConfig == nullptr) return;

		std::string option = globalConfig->get<std::string>(CONFIGURATION_ROOT_DIRECTORY_KEY);

		if (option == "") return;

		std::filesystem::path root(option);

		if (!(std::filesystem::exists(root) && std::filesystem::is_directory(root)))
		{
			throw_with_trace(std::runtime_error("root directory isn't a valid directory: " + root.generic_string()));
		}

		std::filesystem::path canonicalPath = canonical(root);

		// we add a trailing slash so that it is later easier to construct subdirectories
		mRoot = canonicalPath.generic_string() + "/";
	}

	const std::string& Globals::getCompiledVobFileExtension() const
	{
		static const std::string extension = ".CVOB";
		return extension;
	}

	const std::string & Globals::getCompiledPbrFileExtension() const
	{
		static const std::string extension = ".CPROBE";
		return extension;
	}

	const std::string& Globals::getCompiledAnimationFileExtension() const
	{
		static const std::string extension = ".CANI";
		return extension;
	}

	std::string Globals::getResourceDirectoy() const
	{
		return getRootDirectory() + "_work/data/";
	}

	const std::string& Globals::getCompiledRiggedMeshFileExtension() const
	{
		static const std::string extension = ".CMESH_RIGGED";
		return extension;
	}

	const std::string& Globals::getCompiledRigFileExtension() const
	{
		static const std::string extension = ".CRIG";
		return extension;
	}

	const std::string& Globals::getCompiledTextureFileExtension() const
	{
		static const std::string extension = ".CTEX";
		return extension;
	}


	std::string Globals::getCompiledResourceDirectoy() const
	{
		/**
		 * Root folder for compilations
		 */
		static const std::string COMPILED_SUBFOLDER = "_work/data/_compiled/";

		return getRootDirectory() + COMPILED_SUBFOLDER;
	}

	std::string Globals::getEmbeddedTextureFileExtension() const
	{
		return ".EMBEDDED_TEX";
	}

	const std::string& Globals::getMetaFileExtension() const
	{
		static const std::string extension = "_meta.ini";
		return extension;
	}

	std::string Globals::getOpenGLShaderDirectory() const
	{
		return getRootDirectory() + "projects/shaders/opengl/";
	}

	std::string Globals::getInterfaceShaderDirectory() const
	{
		return getRootDirectory() + "projects/shaders/interface/";
	}

	const std::string& Globals::getRootDirectory() const
	{
		return mRoot;
	}
}