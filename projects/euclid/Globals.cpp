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

	std::string Globals::getCompiledTextureDirectory() const
	{
		return getCompiledRootDirectory() + "textures/";
	}

	std::string Globals::getCompiledMeshDirectory() const
	{
		return getCompiledRootDirectory() + "meshes/";
	}

	const std::string& Globals::getCompiledMeshFileExtension() const
	{
		static const std::string extension = ".CMESH";
		return extension;
	}

	std::string Globals::getCompiledPbrDirectory() const
	{
		return getCompiledRootDirectory() + "probes/";
	}

	const std::string & Globals::getCompiledPbrFileExtension() const
	{
		static const std::string extension = ".CPROBE";
		return extension;
	}

	std::string Globals::getCompiledAnimationDirectory() const
	{
		return getCompiledRootDirectory() + "anims/";
	}

	const std::string& Globals::getCompiledAnimationFileExtension() const
	{
		static const std::string extension = ".CANI";
		return extension;
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


	std::string Globals::getCompiledRootDirectory() const
	{
		/**
		 * Root folder for compilations
		 */
		static const std::string COMPILED_SUBFOLDER = "_work/data/_compiled/";

		return getRootDirectory() + COMPILED_SUBFOLDER;
	}

	std::string Globals::getAnimationDirectory() const
	{
		return getRootDirectory() + "_work/data/anims/";
	}

	std::string Globals::getEmbeddedTextureFileExtension() const
	{
		return ".EMBEDDED_TEX";
	}

	std::string Globals::getFontDirectory() const
	{
		return getRootDirectory() + "_work/data/fonts/";
	}

	std::string Globals::getMeshDirectory() const
	{
		return getRootDirectory() + "_work/data/meshes/";
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

	std::string Globals::getTextureDirectory() const
	{
		return getRootDirectory() + "_work/data/textures/";
	}

	const std::string& Globals::getRootDirectory() const
	{
		return mRoot;
	}
}