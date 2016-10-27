#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "mesh/TestMeshes.cpp"
#include <shader/Shader.hpp>
#include <texture/TextureManager.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <camera/Camera.hpp>
#include <platform/Input.hpp>
#include <model/Model.hpp>
#include <memory>
#include <model/ModelFactory.hpp>
#include <shader/SimpleLightShader.hpp>
#include <shader/LampShader.hpp>
#include <shader/PlaygroundShader.hpp>
#include <platform/Window.hpp>
#include <platform/windows/PlatformWindows.hpp>
#include <platform/windows/window/WindowWin32.hpp>
#include <thread>
#include <renderer/RendererOpenGL.hpp>
#include <functional>
#include <filesystem>

using namespace glm;
using namespace std;
using namespace experimental::filesystem;

static void handleWindowEvents(Window* window);
static void doUserMovement(Input* input, double deltaTime);
static void updateMixValueByInput(Input* input);
// Holds uniform value of texture mix
GLfloat mixValue = 0.2f;
Camera camera;

class TestCallback
{
public:
	TestCallback(string name)
	{
		this->name = name;
	}

	void callback(Window* window, bool hasFocus)
	{
		cout << "TestCallback called: " << name << ", params: window = " << window << ", hasFocus = " << hasFocus << endl;
		window->removeWindowFocusCallback(connection);
	}

	void setCallbackConnection(Window::WindowFocusConnection connection)
	{
		this->connection = connection;
	}

	Window::WindowFocusConnection getConnection()
	{
		return connection;
	}

private:
	string name;
	Window::WindowFocusConnection connection;
};

int var = 0;
Window::WindowFocusConnection con0;

void callback(Window* window, bool hasFocus)
{
	cout << "callback called!" << endl;
	++var;
	if (var == 5)
	{
		window->removeWindowFocusCallback(con0);
	}
	if (hasFocus)
	{
		//window->setFullscreen();
	}
	else
	{
		//window->setWindowed();
	}
}

int main(int argv, char** args)
{
	path myPath = "./hello";
	myPath = canonical(myPath, current_path());
	cout << "myPath = " << myPath << endl;
	Window::WindowStruct desc;
	desc.title = "WindowWin32 test";
	desc.fullscreen = false;
	desc.colorBitDepth = 32;
	desc.refreshRate = 60;
	desc.posX = 0;
	desc.posY = 0;
	desc.width = 800;
	desc.height = 600;
	desc.visible = true;
	
	Window* window = new WindowWin32(desc);
	//window->setWindowed();

	Renderer* renderer = new RendererOpenGL();

	window->embedRenderer(renderer);

	window->activate();

	TestCallback test ("hello world!");
	TestCallback test2 ("hello world2!");

	auto callback = std::bind(&TestCallback::callback, &test,
		placeholders::_1, placeholders::_2);

	auto callback2 = std::bind(&TestCallback::callback, &test2,
		placeholders::_1, placeholders::_2);

	con0 = window->addWindowFocusCallback(::callback);
	Window::WindowFocusConnection con1 = window->addWindowFocusCallback(callback);
	test.setCallbackConnection(con1);
	test2.setCallbackConnection(window->addWindowFocusCallback(callback2));

	//window->setFullscreen();
	//window->setWindowed();

	while (window->isOpen())
	{
		window->pollEvents();

		Input* input = window->getInputDevice();
		if (input->isPressed(Input::KeyEscape))
		{
			cout << "Escape is pressed!" << endl;
		}

		if (input->isPressed(Input::KeyKpAdd))
		{
			cout << "Numpad add key is pressed!" << endl;
		}

		if (input->isPressed(Input::KeyEnter))
		{
			cout << "Enter is pressed!" << endl;
		}

		if (input->isPressed(Input::KeyReturn))
		{
			cout << "Return is pressed!" << endl;
		}

		/*if (input->isDown(Input::KeyEscape))
		{
			cout << "Escape is down!" << endl;
		}*/

		if (input->isReleased(Input::KeyEscape))
		{
			cout << "Escape is released!" << endl;
		}

		if (input->isDown(Input::ScrollWheelUp))
		{
			cout << "scrolling up is done!" << endl;
		}

		if (input->isPressed(Input::ScrollWheelUp))
		{
			cout << "scrolling up is done!" << endl;
		}

		if (input->isReleased(Input::ScrollWheelUp))
		{
			cout << "scrolling up is done!" << endl;
		}

		if (input->isDown(Input::ScrollWheelDown))
		{
			cout << "scrolling down is done!" << endl;
		}

		if (input->isPressed(Input::RightMouseButton))
		{
			cout << "RightMouseButton  is down!" << endl;
		}

		if (input->isPressed(Input::LeftMouseButton))
		{
			cout << "LeftMouseButton  is pressed!" << endl;
		}

		if (input->isReleased(Input::LeftMouseButton))
		{
			cout << "LeftMouseButton  is released!" << endl;
		}

		if (input->isPressed(Input::MiddleMouseButton))
		{
			cout << "MiddleMouseButton  is down!" << endl;
		}

		Input::Key anyKey = input->getAnyPressedKey();
		Input::Button anyButton = input->getAnyPressedButton();
		if (anyKey != Input::InvalidKey)
		{
			cout << "any key is pressed: " << anyKey << endl;
		}

		if (anyButton != Input::InvalidButton)
		{
			cout << "any button is pressed: " << anyButton << endl;
		}

		if (window->hasFocus())
		{
			//cout << "Window has focus!" << endl;
		} else
		{
			//cout << "Window lost focus!" << endl;
		}

		MouseOffset offset = input->getFrameMouseOffset();
		//if (!(offset.xOffset == 0 && offset.yOffset == 0))
		//	cout << "Mouse offset: (" << offset.xOffset << ", " << offset.yOffset << ")" << endl;

		handleWindowEvents(window);
		updateMixValueByInput(input);
		doUserMovement(input, 0);

		renderer->beginScene();
		renderer->endScene();
		renderer->present();
		window->swapBuffers();
	}

	cout << "main(): terminating..." << endl;
	delete window;
	window = nullptr;
	delete renderer;
	renderer = nullptr;

	return EXIT_SUCCESS;
}


static void handleWindowEvents(Window* window)
{
	Input* input = window->getInputDevice();

	if (input->isPressed(Input::KeyEscape))
		window->close();

	//TODO minimize window if enter/return was pressed!
	//if (input->isPressed(GLFW_KEY_ENTER) || input->isKeyPressed(GLFW_KEY_KP_ENTER))
	//	glfwIconifyWindow(window);
}

static void updateMixValueByInput(Input* input)
{
	if (input->isPressed(Input::KeyUp))
	{
		mixValue += 0.1f;
		if (mixValue >= 1.0f)
			mixValue = 1.0f;

		mixValue = std::round(mixValue * 10) / 10;
		cout << "MixValue: " << mixValue << endl;
	}

	if (input->isPressed(Input::KeyDown))
	{
		mixValue -= 0.1f;
		if (mixValue <= 0.0f)
			mixValue = 0.0f;

		mixValue = std::round(mixValue * 10) / 10;

		cout << "MixValue: " << mixValue << endl;
	}

}

static void doUserMovement(Input* input, double deltaTime)
{
	// camera movements
	GLfloat cameraSpeed = 5.0f * deltaTime;
	vec3 cameraPos = camera.getPosition();
	vec3 cameraLook = camera.getLookDirection();
	vec3 cameraUp = camera.getUpDirection();
	vec3 cameraRight = normalize(cross(cameraLook, cameraUp));

	if (input->isDown(Input::KeyW))
		cameraPos += cameraSpeed * cameraLook;

	if (input->isDown(Input::KeyS))
		cameraPos -= cameraSpeed * cameraLook;

	if (input->isDown(Input::KeyD))
		cameraPos += cameraSpeed * cameraRight;

	if (input->isDown(Input::KeyA))
		cameraPos -= cameraSpeed * cameraRight;

	camera.setPosition(cameraPos);
	camera.setLookDirection(cameraLook);
	camera.setUpDirection(cameraUp);
}