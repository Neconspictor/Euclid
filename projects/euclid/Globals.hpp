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

		std::string getCompiledMeshDirectory() const;
		const std::string& getCompiledVobFileExtension() const;
		
		std::string getCompiledPbrDirectory() const;
		const std::string& getCompiledPbrFileExtension() const;

		std::string getCompiledAnimationDirectory() const;
		const std::string& getCompiledAnimationFileExtension() const;

		std::string getCompiledResourceDirectoy() const;
		std::string getResourceDirectoy() const;

		const std::string& getCompiledRiggedMeshFileExtension() const;
		const std::string& getCompiledRigFileExtension() const;
		
		std::string getCompiledTextureDirectory() const;
		const std::string& getCompiledTextureFileExtension() const;

		/**
		 * Path to the animations folder. Path ends with a slash
		 */
		std::string getAnimationDirectory() const;

		/**
		 * File extension for embedded textures
		 */
		std::string getEmbeddedTextureFileExtension() const;

		/**
		 * Path to the animations folder. Path ends with a slash
		 */
		std::string getFontDirectory() const;

		/**
		 * Path to the meshes folder. Path ends with a slash
		 */
		std::string getMeshDirectory() const;

		/**
		 * The file extension for meta files.
		 */
		const std::string& getMetaFileExtension() const;

		/**
		* Path to the opengl shader folder. Path ends with a slash
		*/
		std::string getOpenGLShaderDirectory() const;

		/**
		* Path to engine-shader interface folder. Path ends with a slash
		*/
		std::string getInterfaceShaderDirectory() const;

		/**
		* Path to the texture folder. Path ends with a slash
		*/
		std::string getTextureDirectory() const;

		/**
		* Path to the root directory. Path ends with a slash
		*/
		const std::string& getRootDirectory() const;

	private:
		std::string mRoot;
	};
}