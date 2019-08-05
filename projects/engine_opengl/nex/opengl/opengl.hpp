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