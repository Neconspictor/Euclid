#pragma once
#include <platform/Input.hpp>
#include <vector>
#include <SDL/SDL.h>

/**
 * An input device for a SDL context.
 */
class InputDeviceSDL : public Input
{
public:

	InputDeviceSDL();

	virtual ~InputDeviceSDL();

	bool isDown(Key key) override;
	bool isDown(Button button) override;
	bool isPressed(Key key) override;
	bool isPressed(Button button) override;
	bool isReleased(Key key) override;
	bool isReleased(Button button) override;
	Key getAnyPressedKey() override;
	Button getAnyPressedButton() override;

	void handleEvents();

	void pollEvents();

	void push(SDL_Event event);

private:

	static const int MOUSE_BUTTON_SIZE = 5;

	std::vector<SDL_Event> eventQueue;

	InputItemState pressedKeys[SDL_NUM_SCANCODES];
	InputItemState pressedMouseButtons[MOUSE_BUTTON_SIZE];
	std::vector<int> tempStates, tempMouseStates;
	Key anyPressedKey;
	Button anyPressedMouseButton;

	int mapKey(Key key);
	int mapButton(Button button);
};