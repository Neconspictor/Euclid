#ifndef ENGINE_UTIL_GLOBAL_PATHS
#define ENGINE_UTIL_GLOBAL_PATHS
#include <string>

namespace util {
	namespace globals {
		/**
		 * Path to textures
		 */
		static const std::string TEXTURE_PATH = "./_work/data/textures/";

		/**
		* Global shader path
		*/
		static const std::string SHADER_PATH = "./shaders/";

		/**
		* Path to opengl shaders
		*/
		static const std::string SHADER_PATH_OPENGL = SHADER_PATH + "opengl/";
	}
}
#endif