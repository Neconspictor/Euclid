#include <platform/window_system/SDL/InputDeviceSDL.hpp>

using namespace std;
using namespace platform;

InputDeviceSDL::InputDeviceSDL() :
	Input()
{
	logClient.setPrefix("[InputDeviceSDL]");
	/*
	try
	{
		sdl->setWindow(SDL_CreateWindowFrom(window));
		SDL_Window* pSampleWin = sdl->getWindow();
		char sBuf[32];
		sprintf_s<32>(sBuf, "%p", pSampleWin);
		SDL_SetHint(SDL_HINT_VIDEO_WINDOW_SHARE_PIXEL_FORMAT, sBuf);

		if (sdl->getWindow())
		{
			LOG(logClient, Debug) << "SDLInputDevice(): window isn't null!";
		}

		SDL_PumpEvents();

		init = true;
		
	} catch(const SDLInitError& err)
	{
		LOG(logClient, Error)
			<< "Error while initializing SDL:  "
			<< err.what();
		sdl = nullptr;
	}
	*/

	for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
	{
		pressedKeys[i] = Up;
	}

	for (int i = 0; i < MOUSE_BUTTON_SIZE; ++i)
	{
		pressedMouseButtons[i] = Up;
	}

	anyPressedKey = KEY_UNKNOWN;
	anyPressedMouseButton = InvalidButton;
}

InputDeviceSDL::~InputDeviceSDL()
{
}

bool InputDeviceSDL::isDown(Key key)
{
	const Uint8* keys = SDL_GetKeyboardState(nullptr);
	int scancode = mapKey(key);
	if (keys[scancode]) {
		return true;
	}
	return false;
}

bool InputDeviceSDL::isDown(Button button)
{
	int code = mapButton(button);
	if (code < 0) return false;

	InputItemState state = pressedMouseButtons[code];
	return (state == Down) || (state == Pressed);
}

bool InputDeviceSDL::isPressed(Key key)
{
	int scancode = mapKey(key);
	return pressedKeys[scancode] == Pressed;
}

bool InputDeviceSDL::isPressed(Button button)
{
	int scancode = mapButton(button);
	return pressedMouseButtons[scancode] == Pressed;
}

bool InputDeviceSDL::isReleased(Key key)
{
	int scancode = mapKey(key);
	return pressedKeys[scancode] == Released;
}

bool InputDeviceSDL::isReleased(Button button)
{
	int scancode = mapButton(button);
	return pressedMouseButtons[scancode] == Released;
}

Input::Key InputDeviceSDL::getAnyPressedKey()
{
	return anyPressedKey;
}

Input::Button InputDeviceSDL::getAnyPressedButton()
{
	return anyPressedMouseButton;
}

void InputDeviceSDL::handleEvents()
{
	vector<int> currentStateChanges,
		currentMouseStateChanges;
	frameScrollOffsetX = 0;
	frameScrollOffsetY = 0;
	frameMouseXOffset = frameMouseYOffset = 0;
	anyPressedKey = KEY_UNKNOWN;
	anyPressedMouseButton = InvalidButton;

	while(eventQueue.size() > 0)
	{
		SDL_Event event = eventQueue.back();
		eventQueue.pop_back();

		switch (event.type) {
			case SDL_KEYDOWN: {
				InputItemState current = pressedKeys[event.key.keysym.scancode];
				if (current != Pressed && current != Down)
				{
					pressedKeys[event.key.keysym.scancode] = Pressed;
					tempStates.push_back(event.key.keysym.scancode);
					currentStateChanges.push_back(event.key.keysym.scancode);
				}
				break;
			}
			case SDL_KEYUP: {
				pressedKeys[event.key.keysym.scancode] = Released;
				tempStates.push_back(event.key.keysym.scancode);
				currentStateChanges.push_back(event.key.keysym.scancode);
				break;
			}
			case SDL_MOUSEWHEEL: {
				frameScrollOffsetX += event.wheel.x;
				frameScrollOffsetY += event.wheel.y;
				break;
			}
			case SDL_MOUSEMOTION: {
				// only if the absolut mouse position hasn't changed
				// a relative motion makes sense!
				frameMouseXOffset = event.motion.x - mouseXabsolut;
				frameMouseYOffset = event.motion.y - mouseYabsolut;

				mouseXabsolut = event.motion.x;
				mouseYabsolut = event.motion.y;
				break;

			}
			case SDL_MOUSEBUTTONUP: {
				pressedMouseButtons[event.button.button - 1] = Released;
				currentMouseStateChanges.push_back(event.button.button - 1);
				tempMouseStates.push_back(event.button.button - 1);
				break;
			}
			case SDL_MOUSEBUTTONDOWN: {
				pressedMouseButtons[event.button.button - 1] = Pressed;
				currentMouseStateChanges.push_back(event.button.button - 1);
				tempMouseStates.push_back(event.button.button - 1);
				break;
			}
		}
	}

	for (auto it = tempStates.begin(); it != tempStates.end(); )
	{
		auto curIt = find(currentStateChanges.begin(), currentStateChanges.end(), *it);
		if (curIt != currentStateChanges.end()) {
			++it;
			continue;
		}

		int scancode = *it;
		InputItemState state = pressedKeys[scancode];
		if (state == Pressed) pressedKeys[scancode] = Down;
		if (state == Released) pressedKeys[scancode] = Up;
		it = tempStates.erase(it);
	}

	// do the same for mouse state changes
	for (auto it = tempMouseStates.begin(); it != tempMouseStates.end(); )
	{
		auto curIt = find(currentMouseStateChanges.begin(), currentMouseStateChanges.end(), *it);
		if (curIt != currentMouseStateChanges.end()) {
			++it;
			continue;
		}

		int scancode = (*it);

		InputItemState state = pressedMouseButtons[scancode];
		if (state == Pressed) {
			pressedMouseButtons[scancode] = Down;
		}
		if (state == Released) {
			pressedMouseButtons[scancode] = Up;
		}
		it = tempMouseStates.erase(it);
	}

	// check if any key is pressed
	for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
	{
		InputItemState state = pressedKeys[i];
		if (state == Pressed || state == Down)
		{
			anyPressedKey = (Key)i;
			break;
		}
	}

	for (int i = 0; i < MOUSE_BUTTON_SIZE; ++i)
	{
		InputItemState state = pressedMouseButtons[i];
		if (state == Pressed || state == Down)
		{
			anyPressedMouseButton = (Button)i;
			break;
		}
	}

	if (frameScrollOffsetY || frameScrollOffsetX)
	{
		informScrollListeners(frameScrollOffsetX, frameScrollOffsetY);
	}
}

void InputDeviceSDL::pollEvents()
{
}

void InputDeviceSDL::push(SDL_Event event)
{
	eventQueue.push_back(move(event));
}

int InputDeviceSDL::mapKey(Key key)
{
	// the key enum table is inherited from the SDL scancode table.
	// So we have an identity mapping!
	return static_cast<int>(key);
}

int InputDeviceSDL::mapButton(Button button)
{
	switch(button)
	{
	case LeftMouseButton: return SDL_BUTTON_LEFT - 1;
	case RightMouseButton: return SDL_BUTTON_RIGHT - 1;
	case MiddleMouseButton: return SDL_BUTTON_MIDDLE - 1;
	case InvalidButton: break;
	default: break;
	}

	return -1;
}