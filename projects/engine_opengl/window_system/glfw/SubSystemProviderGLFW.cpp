#include <window_system/glfw/SubSystemProviderGLFW.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <gui/ImGUI_GL.hpp>
//#include <utf8.h>

using namespace std;


SubSystemProviderGLFW::SubSystemProviderGLFW() : m_isInitialized(false), logClient(platform::getLogServer())
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

	InputGLFW* input = dynamic_cast<InputGLFW*>(pointer->getInputDevice());
	input->enableCallbacks();

	return pointer;
}

std::unique_ptr<ImGUI_Impl> SubSystemProviderGLFW::createGUI(Window * window)
{
	WindowGLFW& windowGLFW = dynamic_cast<WindowGLFW&>(*window);
	return make_unique<ImGUI_GL>(windowGLFW);
}

Renderer* SubSystemProviderGLFW::getRenderer() {

	//TODO
	return nullptr;
}

Input* SubSystemProviderGLFW::getInput() {
	//TODO
	return nullptr;
}

void SubSystemProviderGLFW::errorCallback(int error, const char* description)
{
	LOG(get()->logClient, platform::Error) << "Error code: " << error
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

	m_isInitialized = (glfwInit() == GLFW_TRUE) ? true : false;

	if (!m_isInitialized) return false;

	glfwSetErrorCallback(errorCallback);
	return true;
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
	
	glfwTerminate();
	m_isInitialized = false;
}