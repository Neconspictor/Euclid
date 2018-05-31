

#include <gui/NanoGUI.hpp>
#include <window_system/glfw/WindowGLFW.hpp>
#include <window_system/glfw/SubSystemProviderGLFW.hpp>

/*#if defined(NANOGUI_GLAD)
#if defined(NANOGUI_SHARED) && !defined(GLAD_GLAPI_EXPORT)
#define GLAD_GLAPI_EXPORT
#endif
*/

#include <glad/glad.h>
/*#else
#if defined(__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#else
#define GL_GLEXT_PROTOTYPES
#endif
#endif*/

#include <glfw/glfw3.h>

//#include <nanogui/nanogui.h>

#include <iostream>

//nanogui::Screen* NanoGUI::screen = nullptr;

double testDouble = 0.3;
std::string strval = "A string";


NanoGUI::NanoGUI(SubSystemProviderGLFW* windowSystem) : windowSystem(windowSystem), window(nullptr)
{
	std::cout << "created NanoGUI" << std::endl;
}

NanoGUI::~NanoGUI()
{
	std::cout << "deleted NanoGUI" << std::endl;
}

void NanoGUI::init(Window* windowSource)
{
	/*window = static_cast<WindowGLFW*>(windowSource);
	std::cout << "NanoGUI initialized!" << std::endl;

	GLFWwindow* source = window->getSource();

	glfwSetTime(0);

	screen = new nanogui::Screen();
	screen->initialize(source, false);


	// Create nanogui gui
	bool enabled = true;
	nanogui::FormHelper *gui = new nanogui::FormHelper(screen);
	nanogui::ref<nanogui::Window> nanoguiWindow = gui->addWindow(Eigen::Vector2i(10, 10), "Form helper example");
	gui->addGroup("Basic types");
	bool test = true;
	gui->addVariable("bool", test)->setTooltip("Test tooltip.");
	gui->addVariable("string", strval);

	gui->addGroup("Validating fields");
	int testInt = 0;
	gui->addVariable("int", testInt)->setSpinnable(true);
	float testFloat = 0.0f;
	gui->addVariable("float", testFloat)->setTooltip("Test.");
	gui->addVariable("double", testDouble)->setSpinnable(true);

	gui->addGroup("Complex types");
	//gui->addVariable("Enumeration", enumval, enabled)->setItems({ "Item 1", "Item 2", "Item 3" });
	gui->addVariable("Color", nanogui::Color(0.5f, 0.5f, 0.7f, 1.f));

	gui->addGroup("Other widgets");
	gui->addButton("A button", []() { std::cout << "Button pressed." << std::endl; })->setTooltip("Testing a much longer tooltip, that will wrap around to new lines multiple times.");;

	screen->setVisible(true);
	screen->performLayout();
	nanoguiWindow->center();

	glfwSetCursorPosCallback(source,
		[](GLFWwindow *, double x, double y) {
		screen->cursorPosCallbackEvent(x, y);
	}
	);

	window->registerMouseCallback([](GLFWwindow *, int button, int action, int modifiers) {
		screen->mouseButtonCallbackEvent(button, action, modifiers);
	});

	window->registerKeyCallback([](GLFWwindow *, int key, int scancode, int action, int mods) {
		screen->keyCallbackEvent(key, scancode, action, mods);
	});

	window->registerCharModsCallback([](GLFWwindow*, unsigned int codepoint, int mods) {
		screen->charCallbackEvent(codepoint);
	});

	glfwSetDropCallback(source,
		[](GLFWwindow *, int count, const char **filenames) {
		screen->dropCallbackEvent(count, filenames);
	}
	);

	window->getInputDevice()->addScrollCallback([](double x, double y) {
		screen->scrollCallbackEvent(x, y);
	});

	window->addResizeCallback([](int width, int height) {
		screen->resizeCallbackEvent(width, height);
	});

	*/
}

void NanoGUI::frameUpdate()
{
	//screen->drawWidgets();
}