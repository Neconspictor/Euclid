// glad has to be included before glfw
#include <nex/opengl/opengl.hpp>
#include <GLFW/glfw3.h>
#include <nex/platform/glfw/WindowGLFW.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <nex/common/Log.hpp>

#ifdef WIN32
#define GLFW_EXPOSE_NATIVE_WIN32 1
#include <GLFW/glfw3native.h>
#endif

void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	std::stringstream ss;
	ss << "0x" << std::hex << id << ": " << message;
	
	LOG(nex::Logger("DebugCallback"), nex::Error) << ss.str();
}


void nex::WindowGLFW::createWindowWithRenderContext()
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	auto* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);


	//glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	//glfwWindowHint(GLFW_SAMPLES, GLFW_DONT_CARE);
	glfwWindowHint(GLFW_REFRESH_RATE, mConfig.refreshRate);



#ifdef EUCLID_ALL_OPTIMIZATIONS
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_FALSE);
	//glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_TRUE);
	glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_FALSE);
#else
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_FALSE);
#endif

	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_FALSE);

	glfwWindowHint(GLFW_VISIBLE, mConfig.visible);

	glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, GLFW_RELEASE_BEHAVIOR_FLUSH);
	glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_NATIVE_CONTEXT_API);


	//glfwWindowHint(GLFW_DECORATED, GL_FALSE);
	//glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	WindowGLFW* windowShared = (WindowGLFW*)mConfig.shared;
	GLFWwindow* shared = nullptr;
	if (mConfig.shared)
		shared = windowShared->getSource();

	window = glfwCreateWindow(mConfig.virtualScreenWidth, mConfig.virtualScreenHeight, mConfig.title.c_str(), nullptr, shared);

	if (!window)
	{
		throw_with_trace(std::runtime_error("(GL)WindowGLFW::createWindowWithRenderContext: Error: Couldn't create GLFWwindow!"));
	}

	activate(true);

	static bool once = false;

	if (!once) {
		once = true;
	} else if (once) {
		//once = true;
		activate();

		// Load all OpenGL functions using the glfw loader function
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
			//if (!gladLoadGL())
		{
			throw_with_trace(std::runtime_error("WindowGLFW::createOpenGLWindow(): Failed to initialize OpenGL context"));
		}

		LOG(mLogger, nex::Info) << "OpenGL version: " << GLVersion.major << "." << GLVersion.minor;

		GLCall(glDebugMessageCallback(DebugCallback, nullptr));
		GLCall(glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE));
		GLCall(glEnable(GL_DEBUG_OUTPUT));

		activate(true);
	}
}