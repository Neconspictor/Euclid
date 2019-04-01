#include <nex/opengl/window_system/glfw/SubSystemProviderGLFW.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <nex/opengl/gui/ImGUI_GL.hpp>
#include <nex/opengl/window_system/glfw/WindowGLFW.hpp>
#include <nex/opengl/window_system/glfw/InputGLFW.hpp>
//#include <utf8.h>

using namespace std;
using namespace nex;


SubSystemProviderGLFW::SubSystemProviderGLFW() : m_isInitialized(false), m_logger("SubSystemProviderGLFW")
{
}

Window* SubSystemProviderGLFW::createWindow(Window::WindowStruct& desc)
{
	WindowGLFW window(desc);
	window.init();

	windows.push_back(move(window));
	WindowGLFW* pointer = &windows.back();

	//pointer->init();

	if (desc.fullscreen)
		pointer->setFullscreen();
	//else
		//pointer->setWindowed();

	nex::InputGLFW* input = dynamic_cast<InputGLFW*>(pointer->getInputDevice());
	input->enableCallbacks();

	return pointer;
}

std::unique_ptr<nex::gui::ImGUI_Impl> SubSystemProviderGLFW::createGUI(Window * window)
{
	return make_unique<gui::ImGUI_GL>(window);
}

void SubSystemProviderGLFW::errorCallback(int error, const char* description)
{
	LOG(get()->m_logger, nex::Error) << "Error code: " << error
		<< ", error description: " << description;
}

SubSystemProviderGLFW* SubSystemProviderGLFW::get()
{
	static SubSystemProviderGLFW instance;
	return &instance;
}

bool SubSystemProviderGLFW::init()
{
	if (m_isInitialized) return true;


	glfwSetErrorCallback(errorCallback);

	m_isInitialized = (glfwInit() == GLFW_TRUE) ? true : false;

	if (!m_isInitialized) return false;

	return true;
}

bool SubSystemProviderGLFW::isTerminated() const
{
	return !m_isInitialized;
}

void SubSystemProviderGLFW::pollEvents()
{
	for (auto& window : windows)
	{
		window.inputDevice.resetForFrame();
	}

	glfwPollEvents();

	// update input device states
	for (auto& window : windows)
	{
		window.inputDevice.frameUpdate();
	}
}

void SubSystemProviderGLFW::terminate()
{
	if (!m_isInitialized) return;
	
	for (auto& window : windows)
	{
		window.close();
		window.release();
	}

	glfwInit();
	glfwTerminate();
	m_isInitialized = false;
}