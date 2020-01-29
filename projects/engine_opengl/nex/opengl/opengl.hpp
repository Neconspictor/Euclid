#pragma once
#include <glad/glad.h>
#include <nex/common/Log.hpp>

extern nex::Logger GLOBAL_RENDERER_LOGGER;

#if defined(EUCLID_ALL_OPTIMIZATIONS)
#define SET_BREAK()
#define ASSERT(x)
#define GLCall(x) x;


#else
#include <nex/common/debug_break.h>
#define SET_BREAK()	 psnip_trap()
#define ASSERT(x) if (!x) {LOG(GLOBAL_RENDERER_LOGGER, nex::LogLevel::Error) << "Assertion failed!"; SET_BREAK();}

// A macro for validating an OpenGL function call.
#define GLCall(x) nex::GLClearError();\
		x;\
		ASSERT(nex::GLLogCall())
#endif



#ifdef USE_CLIP_SPACE_ZERO_TO_ONE
constexpr GLenum EUCLID_OPENGL_CLIP_RANGE = GL_ZERO_TO_ONE;
#else
constexpr GLenum EUCLID_OPENGL_CLIP_RANGE = GL_NEGATIVE_ONE_TO_ONE;
#endif

#ifdef USE_SCREEN_SPACE_UPPER_LEFT_ORIGIN
constexpr GLenum EUCLID_OPENGL_SCREEN_SPACE_ORIGIN = GL_UPPER_LEFT;
#else
constexpr GLenum EUCLID_OPENGL_SCREEN_SPACE_ORIGIN = GL_LOWER_LEFT;
#endif

constexpr unsigned PIXEL_STORE_DEFAULT_ALIGNMENT = 4;

namespace nex
{
	void GLClearError();
	bool GLLogCall();
	void GLDeactivateLog();

	std::string GLErrorToString(GLuint errorCode);

	/**
		* A function for checking any opengl related errors.
		* Mainly intended for debug purposes
		* NOTE: Throws an OpenglException if any opengl related error occured
		* since the last call of glGetError()
		* @param errorPrefix: a prefix that will be put in front of the OpenglException.
		*/
	void checkGLErrors(const std::string& errorPrefix);

	bool checkGLErrorSilently();
}