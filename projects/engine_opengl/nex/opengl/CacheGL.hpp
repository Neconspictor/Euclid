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
		void BindDrawFramebuffer(GLuint framebuffer, bool ignoreErrors = false);

		/**
		 * @param rebind: binds the framebuffer always and ignores cached values
		 */
		void BindFramebuffer(GLuint framebuffer, bool rebind = false);
		void BindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
		void BindReadFramebuffer(GLuint framebuffer, bool ignoreErrors = false);
		void BindTextureUnit(GLuint unit, GLuint texture);

		/**
		 * Queries an integer from the driver that is constant (won't change at runtime).
		 */
		GLint GetConstInteger(GLenum pname);

		static GlobalCacheGL* get();


		GLuint getActiveProgram() const;
		GLuint getActiveDrawFrameBuffer() const;
		GLuint getActiveReadFrameBuffer() const;
		void UseProgram(GLuint program);

	private:
		GLuint mActiveProgram;
		GLuint mActiveDrawFrameBuffer;
		GLuint mActiveReadFrameBuffer;
		std::unordered_map<GLenum, GLint> mConstIntegers;

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

		void Uniform1f(GLint location, GLfloat value);
		void Uniform1i(GLint location, GLint value);
		void Uniform1ui(GLint location, GLuint value);
		void UseProgram();
		

	private:

		GLuint mProgram;
		std::unordered_map<GLint, GLfloat> mUniform1fValues;
		std::unordered_map<GLint, GLint> mUniform1iValues;
		std::unordered_map<GLint, GLuint> mUniform1uiValues;
		

		void assertActiveProgram();
	};
}