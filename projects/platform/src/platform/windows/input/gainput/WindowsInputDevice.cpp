#include "platform/windows/input/gainput/WindowsInputDevice.hpp"
#include <platform/windows/input/gainput/HoldResetGesture.hpp>
#include <iostream>

using namespace std;

WindowsInputDevice::~WindowsInputDevice() {}

WindowsInputDevice::WindowsInputDevice(int width, int height)
{
	manager = new gainput::InputManager();
	manager->SetDisplaySize(width, height);
	this->width = width;
	this->height = height;
	mouseId = manager->CreateDevice<gainput::InputDeviceMouse>(gainput::InputDevice::AutoIndex, gainput::InputDevice::DV_STANDARD);
	//keyboardId = manager->CreateDevice<gainput::InputDeviceKeyboard>(gainput::InputDevice::AutoIndex,gainput::InputDevice::DV_STANDARD);
	//keyMap = new gainput::InputMap(*manager);
	buttonMap = new gainput::InputMap(*manager);
	/*keyMap->MapBool(KeyEscape, keyboardId, gainput::KeyEscape);
	keyMap->MapBool(KeyW, keyboardId, gainput::KeyW);
	keyMap->MapBool(KeyA, keyboardId, gainput::KeyA);
	keyMap->MapBool(KeyS, keyboardId, gainput::KeyS);
	keyMap->MapBool(KeyD, keyboardId, gainput::KeyD);
	keyMap->MapBool(KeyUp, keyboardId, gainput::KeyUp);
	keyMap->MapBool(KeyDown, keyboardId, gainput::KeyDown);
	keyMap->MapBool(KeyEnter, keyboardId, gainput::KeyReturn);
	keyMap->MapBool(KeyEnter, keyboardId, gainput::KeyKpEnter);

	keyMap->MapBool(KeyKpAdd, keyboardId, gainput::KeyKpAdd);
	keyMap->MapBool(KeyKpAdd, keyboardId, gainput::KeyAcute);

	cout << gainput::KeyKpEnter << endl;
	cout << gainput::KeyKpAdd << endl;
	cout << gainput::KeyAcute << endl;
	keyMap->MapBool(KeyReturn, keyboardId, gainput::KeyReturn);*/
	
	buttonMap->MapBool(RightMouseButton, mouseId, gainput::MouseButtonRight);
	buttonMap->MapBool(LeftMouseButton, mouseId, gainput::MouseButtonLeft);
	buttonMap->MapBool(MiddleMouseButton, mouseId, gainput::MouseButtonMiddle);
	//buttonMap->MapBool(ButtonConfirm, mouseId, gainput::MouseButtonRight);
	//buttonMap->MapFloat(MouseX, mouseId, gainput::MouseAxisX);
	//buttonMap->MapFloat(MouseY, mouseId, gainput::MouseAxisY);

	//buttonMap->MapBool(ScrollWheelUp, mouseId, gainput::MouseButtonWheelUp);
	//buttonMap->MapBool(ScrollWheelDown, mouseId, gainput::MouseButtonWheelDown);

	buttonMap->MapFloat(ScrollWheelUp, mouseId, gainput::MouseButtonWheelUp);
	buttonMap->MapFloat(ScrollWheelDown, mouseId, gainput::MouseButtonWheelDown);

	/*gainput::HoldResetGesture* hg = manager->CreateAndGetDevice<gainput::HoldResetGesture>();
	GAINPUT_ASSERT(hg);
	hg->Initialize(mouseId, gainput::Touch0Down,
		mouseId, gainput::Touch0X, 0.1f,
		mouseId, gainput::Touch0Y, 0.1f,
		false,
		800);
	buttonMap->MapBool(HoldResetButton, hg->GetDeviceId(), gainput::HoldTriggered);*/

//	map->MapBool(ButtonConfirm, mouseId, gainput::TapTriggered);
	//map.MapBool(ButtonConfirm, padId, gainput::PadButtonA);
}

void WindowsInputDevice::handleMessage(MSG const& msg)
{
	// Forward any input messages to Gainput
	if (msg.message == WM_MOUSEWHEEL)
	{
		cout << "WindowsInputDevice::handleMessage: WM_MOUSEWHEEL received!" << endl;
	}
	manager->HandleMessage(msg);
}

void WindowsInputDevice::update()
{
	manager->Update();
}

bool WindowsInputDevice::isDown(Key key)
{
	return buttonMap->GetBool(key);
}

bool WindowsInputDevice::isPressed(Key key)
{
	return buttonMap->GetBoolIsNew(key);
}

bool WindowsInputDevice::isReleased(Key key)
{
	return buttonMap->GetBoolWasDown(key);
}

bool WindowsInputDevice::isDown(Button button)
{
	//manager->GetDevice(mouseId)->GetBool(gainput::MouseButtonWheelUp);
	return buttonMap->GetBool(button);
}

bool WindowsInputDevice::isPressed(Button button)
{
	
	return buttonMap->GetBoolIsNew(button);
}

bool WindowsInputDevice::isReleased(Button button)
{
	return buttonMap->GetBoolWasDown(button);
}


Input::Button WindowsInputDevice::getAnyPressedButton()
{
	gainput::DeviceButtonSpec pressedButtons[10] = { 0 };
	int size = manager->GetDevice(mouseId)->GetAnyButtonDown(pressedButtons, 10);
	if (size > 0)
	{
		gainput::InputDeviceMouse* mouseDevice = (gainput::InputDeviceMouse*)manager->GetDevice(mouseId);
		mouseDevice->SetSynced(true);
		return mapToButton(pressedButtons[0].buttonId);
	}
	return InvalidButton;
}

Input::Key WindowsInputDevice::getAnyPressedKey()
{
	gainput::DeviceButtonSpec pressedButtons[10] = { 0 };
	int size = manager->GetDevice(keyboardId)->GetAnyButtonDown(pressedButtons, 10);
	if (size > 0)
	{
	
		gainput::InputDeviceKeyboard* keyboardDevice = (gainput::InputDeviceKeyboard*)manager->GetDevice(keyboardId);
		keyboardDevice->SetTextInputEnabled(true);
		return mapToKey(pressedButtons[0].buttonId);
	}
	return InvalidKey;
}

Input::Key WindowsInputDevice::mapToKey(gainput::DeviceButtonId id)
{
	switch (id)
	{
	case gainput::KeyW:
		return KeyW;
	case gainput::KeyS:
		return KeyS;
	case gainput::KeyA:
		return KeyA;
	case gainput::KeyD:
		return KeyD;
	case gainput::KeyReturn:
		return KeyReturn;
	case gainput::KeyEscape:
		return KeyEscape;
	}

	return InvalidKey;
}

Input::Button WindowsInputDevice::mapToButton(gainput::DeviceButtonId id)
{
	switch (id)
	{
	case gainput::MouseButtonLeft:
		return LeftMouseButton;
	case gainput::MouseButtonWheelUp:
		return ScrollWheelUp;
	case gainput::MouseButtonWheelDown:
		return ScrollWheelDown;
	}

	return InvalidButton;
}