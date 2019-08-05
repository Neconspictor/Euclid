#include <nex/opengl/opengl.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/exception/OpenglException.hpp>

nex::Logger GLOBAL_RENDERER_LOGGER("Global Renderer");

namespace nex
{

	bool glLogDeactivated = false;

	void GLClearError()
	{
		if (glLogDeactivated) return;
		unsigned int finite = 4096;
		GLuint errorCode = glGetError();

		while (errorCode && finite)
		{
			if (errorCode == GL_INVALID_OPERATION)
				--finite;
			errorCode = glGetError();
		};

		if (!finite)
		{
			static nex::Logger logger("[GLClearError]");
			logger.log(nex::LogLevel::Warning) << "Detected to many GL_INVALID_OPERATION errors. Assuming that no valid OpenGL context exists.";
			//LOG(logger) << "Detected to many GL_INVALID_OPERATION errors. Assuming that no valid OpenGL context exists.";
			throw_with_trace(std::runtime_error("Detected to many GL_INVALID_OPERATION errors"));
		}
	}

	bool GLLogCall()
	{
		static nex::Logger logger("OpenGL Error");

		bool noErrorsOccurred = true;

		if (glLogDeactivated) return noErrorsOccurred;

		while (GLenum error = glGetError())
		{
			logger.log(nex::LogLevel::Warning) << "Error occurred: " << GLErrorToString(error) << " (0x" << std::hex << error << ")";
			noErrorsOccurred = false;
		}

		return noErrorsOccurred;
	}

	void GLDeactivateLog()
	{
		glLogDeactivated = true;
	}

	std::string GLErrorToString(GLuint errorCode)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		default:							   error = "Unknown error code: " + std::to_string(errorCode);
		}

		return error;
	}

	void checkGLErrors(const std::string& errorPrefix)
	{
		return;
		// check if any gl related errors occured
		GLint errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			std::string error;
			switch (errorCode)
			{
			case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
			case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
			case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
			case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
			default:							   error = "Unknown error code: " + std::to_string(errorCode);
			}

			std::stringstream ss; ss << errorPrefix << ": Error occured: " << error;
			throw_with_trace(OpenglException(ss.str()));
		}
	}

	bool checkGLErrorSilently()
	{
		// check if any gl related errors occured
		GLint errorCode = glGetError();
		if (errorCode != GL_NO_ERROR)
		{
			std::string error;
			switch (errorCode)
			{
			case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
			case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
			case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
			case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
			default:							   error = "Unknown error code: " + std::to_string(errorCode);
			}

			return true;
		}

		return false;
	}
}