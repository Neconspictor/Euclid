#include <platform/windows/input/SDL/SDLInputDevice.hpp>
#include <iostream>

using namespace std;

SDLInputDevice::SDLInputDevice(HWND window, int width, int height)
{
	init = false;
	try
	{
		sdl = new SDL(SDL_INIT_EVERYTHING);
		//SDL_SetHint(SDL_HINT_VIDEO_WINDOW_SHARE_PIXEL_FORMAT, );
		SDL_Window* pSampleWin = SDL_CreateWindow("", 0, 0, 1, 1, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);

		char sBuf[32];
		sprintf_s<32>(sBuf, "%p", pSampleWin);

		SDL_SetHint(SDL_HINT_VIDEO_WINDOW_SHARE_PIXEL_FORMAT, sBuf);

		sdl->setWindow(SDL_CreateWindowFrom(window));
		SDL_SetHint(SDL_HINT_VIDEO_WINDOW_SHARE_PIXEL_FORMAT, nullptr);

		SDL_GLContext glcontext = SDL_GL_CreateContext(sdl->getWindow());
		if (glcontext == NULL) cerr << "SDL_GLContext is null!" << endl;
		if (sdl->getWindow())
		{
			cout << "SDLInputDevice::SDLInputDevice(): window isn't null!" << endl;
		}

		SDL_DestroyWindow(pSampleWin);

		SDL_PumpEvents();

		init = true;
		
	} catch(const InitError& err)
	{
		cerr
			<< "Error while initializing SDL:  "
			<< err.what() << endl;
		sdl = nullptr;
	}

	for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
	{
		pressedKeys[i] = Up;
	}

	for (int i = 0; i < MOUSE_BUTTON_SIZE; ++i)
	{
		pressedMouseButtons[i] = Up;
	}

	anyPressedKey = InvalidKey;
	anyPressedMouseButton = InvalidButton;
}

SDLInputDevice::~SDLInputDevice()
{
	if (sdl) delete sdl;
	sdl = nullptr;

	cout << "SDL is shut down!" << endl;
}

bool SDLInputDevice::isDown(Key key)
{
	const Uint8* keys = SDL_GetKeyboardState(nullptr);
	int scancode = mapKey(key);
	if (keys[scancode]) {
		return true;
	}
	return false;
}

bool SDLInputDevice::isDown(Button button)
{
	if (button == ScrollWheelDown) return frameScrollOffset < 0;
	if (button == ScrollWheelUp) return frameScrollOffset > 0;

	int code = mapButton(button);
	if (code < 0) return false;
	return pressedMouseButtons[code];
}

bool SDLInputDevice::isPressed(Key key)
{
	int scancode = mapKey(key);
	return pressedKeys[scancode] == Pressed;
}

bool SDLInputDevice::isPressed(Button button)
{
	int scancode = mapButton(button);
	return pressedMouseButtons[scancode] == Pressed;
}

bool SDLInputDevice::isReleased(Key key)
{
	int scancode = mapKey(key);
	return pressedKeys[scancode] == Released;
}

bool SDLInputDevice::isReleased(Button button)
{
	int scancode = mapButton(button);
	return pressedMouseButtons[scancode] == Released;
}

Input::Key SDLInputDevice::getAnyPressedKey()
{
	return anyPressedKey;
}

Input::Button SDLInputDevice::getAnyPressedButton()
{
	return anyPressedMouseButton;
}

bool SDLInputDevice::isInit() const
{
	return init;
}

void SDLInputDevice::pollEvents()
{
	SDL_PumpEvents();
	SDL_Event event;
	vector<int> currentStateChanges,
		currentMouseStateChanges;
	bool gotEvents = false;
	frameScrollOffset = 0;
	frameMouseXOffset = frameMouseYOffset = 0;
	anyPressedKey = InvalidKey;
	anyPressedMouseButton = InvalidButton;

	while (SDL_PollEvent(&event))
	{
		gotEvents = true;
		switch (event.type) {
			case SDL_KEYDOWN: {
				KeyState current = pressedKeys[event.key.keysym.scancode];
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
				frameScrollOffset += event.wheel.y;
				cout << "frameScrollOffset = " << frameScrollOffset << endl;
				break;
			}
			case SDL_MOUSEMOTION: {
				frameMouseXOffset = event.motion.xrel;
				frameMouseYOffset = event.motion.yrel;
				mouseXabsolut = event.motion.x;
				mouseYabsolut = event.motion.y;
				break;
					 
			}
			case SDL_MOUSEBUTTONUP: {
				pressedMouseButtons[event.button.button - 1] = Released;
				currentMouseStateChanges.push_back(event.button.button -1);
				tempMouseStates.push_back(event.button.button -1);
				break;
			}
			case SDL_MOUSEBUTTONDOWN: {
				pressedMouseButtons[event.button.button - 1] = Pressed;
				currentMouseStateChanges.push_back(event.button.button -1);
				tempMouseStates.push_back(event.button.button -1);
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
		KeyState state = pressedKeys[scancode];
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

		KeyState state = pressedMouseButtons[scancode];
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
		KeyState state = pressedKeys[i];
		if (state == Pressed || state == Down)
		{
			anyPressedKey = (Key)i;
			break;
		}
	}

	for (int i = 0; i < MOUSE_BUTTON_SIZE; ++i)
	{
		KeyState state = pressedMouseButtons[i];
		if (state == Pressed || state == Down)
		{
			anyPressedMouseButton = (Button)i;
			break;
		}
	}
}

int SDLInputDevice::mapKey(Key key)
{
	switch(key)
	{
	case KeyW:return SDL_SCANCODE_W;
	case KeyA: return SDL_SCANCODE_A;
	case KeyS: return SDL_SCANCODE_S;
	case KeyD: return SDL_SCANCODE_D;
	case KeyEscape: return SDL_SCANCODE_ESCAPE;
	case KeyUp: return SDL_SCANCODE_UP;
	case KeyDown: return SDL_SCANCODE_DOWN;
	case KeyEnter: return SDL_SCANCODE_KP_ENTER;
	case KeyReturn: return SDL_SCANCODE_RETURN;
	case KeyKpAdd: return SDL_SCANCODE_KP_PLUS;
	default: break;
	}
	
	// invalid/unknown key
	return SDL_SCANCODE_UNKNOWN;
}

int SDLInputDevice::mapButton(Button button)
{
	switch(button)
	{
	case LeftMouseButton: return SDL_BUTTON_LEFT - 1;
	case RightMouseButton: return SDL_BUTTON_RIGHT - 1;
	case MiddleMouseButton: return SDL_BUTTON_MIDDLE - 1;
	case ScrollWheelUp: break;
	case ScrollWheelDown: break;
	case InvalidButton: break;
	default: break;
	}

	return -1;
}