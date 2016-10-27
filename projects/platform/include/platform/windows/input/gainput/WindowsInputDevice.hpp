#ifndef WINDOWS_INPUT_DEVICE_HPP
#define WINDOWS_INPUT_DEVICE_HPP

#include "platform//Input.hpp"
#include "platform/windows/PlatformWindows.hpp"
#include <gainput/gainput.h>

class WindowsInputDevice : public Input {
public:
	WindowsInputDevice(int width, int height);

	virtual ~WindowsInputDevice();

	void handleMessage(MSG const& msg);

	void update();

	bool isDown(Key key) override;

	bool isPressed(Key key) override;

	bool isReleased(Key key) override;

	Key getAnyPressedKey() override;

	bool isDown(Button button) override;

	bool isPressed(Button button) override;
	
	bool isReleased(Button button) override;
	
	Button getAnyPressedButton() override;

private:
	int width, height;
	gainput::InputManager* manager;
	gainput::InputMap* keyMap;
	gainput::InputMap* buttonMap;
	gainput::DeviceId mouseId;
	gainput::DeviceId keyboardId;

	Key mapToKey(gainput::DeviceButtonId id);
	Button mapToButton(gainput::DeviceButtonId id);

};

#endif