#pragma once
#include "glad/glad.h"

/**
 * Classes that cache opengl state in order to minimize expensive state changes
 **/

namespace nex
{

	class CacheError : public std::runtime_error
	{
	public:
		explicit CacheError(const std::string& _Message);

		explicit CacheError(const char* _Message);
	};

	class GlobalCacheGL
	{
	public:
		
		void BindTextureUnit(GLuint unit, GLuint texture);
		
		static GlobalCacheGL* get();


		GLuint getActiveProgram() const;
		void UseProgram(GLuint program);

	private:
		GLuint mActiveProgram;

		GlobalCacheGL();
	};

	/**
	 * A cache that is only valid for a specific shader program.
	 * In order to function correctly, the shader program must be bound when using any caching function.
	 *
	 * If EUCLID_ALL_OPTIMIZATIONS is NOT defined:
	 * All caching function will check if the specified shader program is currently active. If not, a CacheError exception is thrown.
	 */
	class ShaderCacheGL {
	public:

		ShaderCacheGL(GLuint program);

		void Uniform1i(GLint location, GLint value);
		void UseProgram();

	private:

		static GlobalCacheGL* mGlobalCache;

		GLuint mProgram;
		std::unordered_map<GLint, GLint> mUniform1iValues;

		void assertActiveProgram();
	};
}