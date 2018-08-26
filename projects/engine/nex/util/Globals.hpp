#pragma once
#include <string>

namespace util {
	class Globals {
	public:
		static void initGlobals();

		/**
		 * Path to the meshes folder. Path ends with a slash
		 */
		static std::string getMeshesPath();

		/**
		* Path to the meshes folder. Path ends with a slash
		*/
		static std::string getOpenGLShaderPath();

		/**
		* Path to the meshes folder. Path ends with a slash
		*/
		static std::string getTexturePath();

		/**
		* Path to the root directory. Path ends with a slash
		*/
		static std::string getRootDirectory();

	private:
		static std::string rootDirectory;

		/**
		* Path to textures
		*/
		static inline const std::string MESHES_PATH = "_work/data/meshes/";

		/**
		* Global shader path
		*/
		static inline const std::string SHADER_PATH = "shaders/";

		/**
		* Path to opengl shaders
		*/
		static inline const std::string SHADER_PATH_OPENGL = SHADER_PATH + "opengl/";

		/**
		* Path to textures
		*/
		static inline const std::string TEXTURE_PATH = "_work/data/textures/";
	};
}
