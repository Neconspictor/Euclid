#include <platform/window_system/glfw/WindowSystemGLFW.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <utf8.h>

using namespace std;

WindowSystemGLFW WindowSystemGLFW::instance;


WindowSystemGLFW::WindowSystemGLFW() : m_isInitialized(false), logClient(platform::getLogServer())
{
}

Window* WindowSystemGLFW::createWindow(Window::WindowStruct& desc)
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

Renderer* WindowSystemGLFW::getRenderer() {

	//TODO
	return nullptr;
}

Input* WindowSystemGLFW::getInput() {
	//TODO
	return nullptr;
}

void WindowSystemGLFW::errorCallback(int error, const char* description)
{
	LOG(instance.logClient, platform::Error) << "Error code: " << error
		<< ", error description: " << description;
}

WindowSystemGLFW* WindowSystemGLFW::get()
{
	return &instance;
}

bool WindowSystemGLFW::init()
{
	if (m_isInitialized) return true;

	m_isInitialized = (glfwInit() == GLFW_TRUE) ? true : false;

	if (!m_isInitialized) return false;

	glfwSetErrorCallback(errorCallback);
	return true;
}

void WindowSystemGLFW::pollEvents()
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

void WindowSystemGLFW::terminate()
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