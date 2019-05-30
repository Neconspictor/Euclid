#pragma once
#include <string>

namespace nex {
	class Configuration;
}

namespace nex::util {
	class Globals {
	public:

		static constexpr const char* CONFIGURATION_ROOT_DIRECTORY_KEY = "General.rootDirectory";

		void init(Configuration* globalConfig);

		std::string getCompiledMeshFolder();
		const std::string& getCompiledMeshFileExtension();
		std::string getCompiledPbrFolder();
		const std::string& getCompiledTextureFileExtension();

		std::string getCompiledRootFolder();

		/**
		 * Path to the meshes folder. Path ends with a slash
		 */
		std::string getMeshesPath();

		/**
		* Path to the meshes folder. Path ends with a slash
		*/
		std::string getOpenGLShaderPath();

		/**
		* Path to the meshes folder. Path ends with a slash
		*/
		std::string getTexturePath();

		/**
		* Path to the root directory. Path ends with a slash
		*/
		const std::string& getRootDirectory();

	private:
		std::string mRoot;
	};
}
