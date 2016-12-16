#pragma once
#include <Windows.h>
#include <platform/Input.hpp>
#include "SDL.hpp"
#include <vector>

/**
 * An input device for a SDL context.
 */
class SDLInputDevice : public Input
{
public:

	SDLInputDevice(HWND windows, int width, int height);

	virtual ~SDLInputDevice();

	bool isDown(Key key) override;
	bool isDown(Button button) override;
	bool isPressed(Key key) override;
	bool isPressed(Button button) override;
	bool isReleased(Key key) override;
	bool isReleased(Button button) override;
	Key getAnyPressedKey() override;
	Button getAnyPressedButton() override;

	bool isInit() const;

	void pollEvents();

private:

	static const int MOUSE_BUTTON_SIZE = 5;
	SDL* sdl;
	InputItemState pressedKeys[SDL_NUM_SCANCODES];
	InputItemState pressedMouseButtons[MOUSE_BUTTON_SIZE];
	std::vector<int> tempStates, tempMouseStates;
	Key anyPressedKey;
	Button anyPressedMouseButton;
	bool init;

	int mapKey(Key key);
	int mapButton(Button button);
};