
#include <window_system/glfw/WindowGLFW.hpp>
#include <gui/ImGUI_GL.hpp>

#include <GLFW/glfw3.h>

#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#endif

// GLFW data
/*static GLFWwindow*  g_Window = NULL;
static double       g_Time = 0.0;
static bool         g_MouseJustPressed[3] = { false, false, false };

// OpenGL3 data
static GLuint       g_FontTexture = 0;
static int          g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;
static int          g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;
static int          g_AttribLocationPosition = 0, g_AttribLocationUV = 0, g_AttribLocationColor = 0;
static unsigned int g_VboHandle = 0, g_ElementsHandle = 0;*/

ImGUI_GL::ImGUI_GL(WindowGLFW& window, std::string glsl_version) : window(&window), glsl_version(move(glsl_version))
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	memset(g_MouseCursors, 0, sizeof(g_MouseCursors));
	std::fill(g_MouseJustPressed, g_MouseJustPressed +3, false);

	init();
}

ImGUI_GL::~ImGUI_GL()
{
	// Destroy GLFW mouse cursors
	for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
		glfwDestroyCursor(g_MouseCursors[cursor_n]);
	memset(g_MouseCursors, 0, sizeof(g_MouseCursors));

	//ImGui_ImplGlfwGL3_InvalidateDeviceObjects  // TODO!!!!!!!!!!!!!!!!!!!!!!!!!!

	ImGui::DestroyContext();
}

void ImGUI_GL::newFrame()
{
}

void ImGUI_GL::renderDrawData(ImDrawData * draw_data)
{
}

void ImGUI_GL::shutdown()
{
}

void ImGUI_GL::init()
{
	// Setup back-end capabilities flags
	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;   // We can honor GetMouseCursor() values (optional)
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;    // We can honor io.WantSetMousePos requests (optional, rarely used)

															// Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
	io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
	io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
	io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
	io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
	io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
	io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
	io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
	io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
	io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
	io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
	io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
	io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
	io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
	io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
	io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
	io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

	io.SetClipboardTextFn = ImGUI_GL::setClipboardText;
	io.GetClipboardTextFn = ImGUI_GL::getClipboardText;
	io.ClipboardUserData = window->getSource();
#ifdef _WIN32
	io.ImeWindowHandle = glfwGetWin32Window(window->getSource());
#endif

	// Load cursors
	// FIXME: GLFW doesn't expose suitable cursors for ResizeAll, ResizeNESW, ResizeNWSE. We revert to arrow cursor for those.
	g_MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

	InputGLFW& input = dynamic_cast<InputGLFW&>(*window->getInputDevice());

	input.registerMouseCallback([&](GLFWwindow* window, int button, int action, int mods) {
		std::cout << "mouse button callback: button: " << button << ", action: " << action << ", mods: " << mods  << std::endl;

		if (action == GLFW_PRESS && button >= 0 && button < 3)
			this->g_MouseJustPressed[button] = true;
	});

	input.addScrollCallback([](double scrollX, double scrollY) {
		std::cout << "scroll callback: scrollX: " << scrollX << ", scrollY: " << scrollY << std::endl;
		ImGuiIO& io = ImGui::GetIO();
		io.MouseWheelH += (float)scrollX;
		io.MouseWheel += (float)scrollY;
	});

	input.registerCharModsCallback([](GLFWwindow* window, unsigned int codepoint, int mods) {
		std::cout << "char mods callback: codepoint: " << codepoint << ", mods: " << mods << std::endl;

		ImGuiIO& io = ImGui::GetIO();
		if (codepoint > 0 && codepoint < 0x10000)
			io.AddInputCharacter((unsigned short)codepoint);
	});

	//void(GLFWwindow*, int, int, int, int);
	input.registerKeyCallback([](GLFWwindow* window, int key, int scancode, int action, int mods) {
		std::cout << "key callback: key=" << key << ", scancode: " << scancode << ", action= " << action << ", mods= " << mods << std::endl;

		ImGuiIO& io = ImGui::GetIO();
		if (action == GLFW_PRESS)
			io.KeysDown[key] = true;
		if (action == GLFW_RELEASE)
			io.KeysDown[key] = false;

		(void)mods; // Modifiers are not reliable across systems
		io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
		io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
		io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
		io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
	});
}

const char* ImGUI_GL::getClipboardText(void* user_data)
{
	return glfwGetClipboardString((GLFWwindow*)user_data);
}

void ImGUI_GL::setClipboardText(void* user_data, const char* text)
{
	glfwSetClipboardString((GLFWwindow*)user_data, text);
}