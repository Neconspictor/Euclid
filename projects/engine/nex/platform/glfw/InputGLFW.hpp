#pragma once

#include <unordered_set>
#include <list>
#include <nex/platform/Input.hpp>

struct GLFWwindow;

namespace nex
{
	class WindowGLFW;


	class InputMapperGLFW {
	public:

		InputMapperGLFW(KeyMapLanguage language);

		Input::Button toButton(int glfwButton) const;
		int toGLFWbutton(Input::Button button) const;
		int toGLFWkey(Input::Key) const;
		Input::Key toKey(int glfwKey) const;

		void setKeyMapLanguage(KeyMapLanguage language);

	protected:
		void initInputButtonMap();
		void initInputKeyMap_US();
		void initInputKeyMap_DE();

	private:
		// mapping glfw button <-> input button
		std::unordered_map<int, Input::Button> glfwToButtonMap;
		std::unordered_map<Input::Button, int> buttonToGlfwMap;

		// mapping glfw key <-> input key
		std::unordered_map<int, Input::Key> glfwToKeyMap;
		std::unordered_map<Input::Key, int> keyToGlfwMap;
	};



	class InputGLFW : public Input
	{
	public:

		using CharModsCallback = void(GLFWwindow*, unsigned int, int);
		using FocusCallback = void(GLFWwindow* window, int hasFocus);
		using KeyCallback = void(GLFWwindow*, int, int, int, int);
		using MouseCallback = void(GLFWwindow* window, int button, int state, int mods);
		using RefreshCallback = void(GLFWwindow* window);
		using ScrollCallback = void(GLFWwindow*, double xOffset, double yOffset);
		using SizeCallback = void(GLFWwindow* window, int width, int height);


		explicit InputGLFW(WindowGLFW* window, KeyMapLanguage language);

		InputGLFW(const InputGLFW&) = delete;
		InputGLFW& operator=(const InputGLFW&) = delete;

		InputGLFW(InputGLFW&& o) noexcept;
		InputGLFW& operator=(InputGLFW&& o) noexcept;


		static void charModsInputHandler(GLFWwindow * window, unsigned int codepoint, int mods);
		static void closeWindowCallbackHandler(GLFWwindow*);

		static void focusInputHandler(GLFWwindow* window, int hasFocus);
		static void keyInputHandler(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouseInputHandler(GLFWwindow * window, int button, int action, int mods);
		static void scrollInputHandler(GLFWwindow* window, double xOffset, double yOffset);
		static void windowSizeInputHandler(GLFWwindow* window, int width, int height);
		static void frameBufferSizeInputHandler(GLFWwindow* window, int width, int height);



		bool areCallbacksActive() const;

		void scrollCallback(double xOffset, double yOffset);

		void disableCallbacks();
		void enableCallbacks();
		void removeCallbacks();

		void frameUpdate();

		Button getAnyPressedButton() const override;
		Key getAnyPressedKey() const override;

		void setClipBoardText(const char* text) override;
		const char* getClipBoardText() const override;

		Window* getWindow() override;
		bool isDown(Button button) const override;
		bool isDown(Key key) const override;
		bool isPressed(Button button) const override;
		bool isPressed(Key key) const override;
		bool isReleased(Button button) const override;
		bool isReleased(Key key) const override;

		void onKey(int key, int scancode, int action, int mods);
		void onMouse(int button, int action, int mods);

		void resetForFrame();

		void setMousePosition(int xPos, int yPos, bool updateOffsets=false) override;
		void setWindow(WindowGLFW* window);

		void setKeyMapLanguage(KeyMapLanguage language) override;

	protected:
		WindowGLFW* window;
		Key anyPressedKey;
		Button anyPressedButton;

		bool _disableCallbacks;

		nex::Logger m_logger;

		// key states
		std::unordered_set<int> downKeys;
		std::unordered_set<int> pressedKeys;
		std::unordered_set<int> releasedKeys;

		// mouse button states
		std::unordered_set<int> downButtons;
		std::unordered_set<int> pressedButtons;
		std::unordered_set<int> releasedButtons;

		InputMapperGLFW mMapper;
	};
}