#include "platform/windows/input/OIS/OISWindowsID.hpp"
#include <OIS/OIS.h>
#include <sstream>
#include <iostream>

using namespace std;

OISWindowsID::OISWindowsID(HWND window, int width, int height)
{
	OIS::ParamList paramList;
	ostringstream windowHndStr;
	windowHndStr << (size_t)window;
	paramList.insert(make_pair(string("WINDOW"), windowHndStr.str()));
	paramList.insert(make_pair(string("w32_mouse"), string("DISCL_FOREGROUND")));
	paramList.insert(make_pair(string("w32_mouse"), string("DISCL_NONEXCLUSIVE")));
	paramList.insert(make_pair(string("w32_keyboard"), string("DISCL_FOREGROUND")));
	paramList.insert(make_pair(string("w32_keyboard"), string("DISCL_NONEXCLUSIVE")));
	m_InputManager = OIS::InputManager::createInputSystem(paramList);
	m_Mouse = static_cast<OIS::Mouse*>(m_InputManager->createInputObject(OIS::OISMouse, false));
	m_Keyboard = static_cast<OIS::Keyboard*>(m_InputManager->createInputObject(OIS::OISKeyboard, false));

	const OIS::MouseState &ms = m_Mouse->getMouseState();
	ms.width = width;
	ms.height = height;

	for (int i = 0; i < KEY_SIZE; ++i)
	{
		pressedKeys[i] = Up;
	}
}

OISWindowsID::~OISWindowsID()
{
}

bool OISWindowsID::isDown(Key key)
{
	OIS::KeyCode code = mapKey(key);
	if (code == OIS::KC_UNASSIGNED) return false;
	bool result = m_Keyboard->isKeyDown(code);
	return result;
}

bool OISWindowsID::isDown(Button button)
{
	OIS::MouseState const& state = m_Mouse->getMouseState();
	if (button == ScrollWheelUp)
	{
		if (state.Z.rel > 0) return true;
		return false;
	} else if (button == ScrollWheelDown)
	{
		if (state.Z.rel < 0) return true;
		return false;
	}
	OIS::KeyCode code = mapButton(button);
	if (code == OIS::KC_UNASSIGNED) return false;
	return false;
}

bool OISWindowsID::isPressed(Key key)
{
	return pressedKeys[key] == Pressed;
}

bool OISWindowsID::isPressed(Button button)
{
	return false;
}

bool OISWindowsID::isReleased(Key key)
{
	return pressedKeys[key] == Released;
}

bool OISWindowsID::isReleased(Button button)
{
	return false;
}

Input::Key OISWindowsID::getAnyPressedKey()
{
	return InvalidKey;
}

Input::Button OISWindowsID::getAnyPressedButton()
{
	return InvalidButton;
}

void OISWindowsID::update()
{
	m_Keyboard->capture();
	m_Mouse->capture();

	//update mouse position
	OIS::MouseState const& state = m_Mouse->getMouseState();
	frameMouseXOffset = state.X.rel;
	frameMouseYOffset = state.Y.rel;
	mouseXabsolut = state.X.abs;
	mouseYabsolut = state.Y.abs;

	//update scroll
	frameScrollOffset = state.Z.rel;

	//update keys
	for (int i = 0; i < KEY_SIZE; ++i)
	{
		Key key = (Key)i;
		OIS::KeyCode code = mapKey(key);
		bool result = m_Keyboard->isKeyDown(code);
		if (result && pressedKeys[key] == Up)
		{
			pressedKeys[key] = Pressed;
		} else if (result && (pressedKeys[key] == Pressed || pressedKeys[key] == Down))
		{
			pressedKeys[key] = Down;
		} else if (!result && pressedKeys[key] == Down)
		{
			pressedKeys[key] = Released;
		} else
		{
			pressedKeys[key] = Up;
		}
		
	}
}

OIS::KeyCode OISWindowsID::mapKey(Key key)
{
	switch(key)
	{
	case KeyW:
		return OIS::KC_W;
	case KeyA:
		return OIS::KC_A;
	case KeyS:
		return OIS::KC_S;
	case KeyD:
		return OIS::KC_D;
	case KeyUp:
		return OIS::KC_UP;
	case KeyDown:
		return OIS::KC_DOWN;
	case KeyEscape:
		return OIS::KC_ESCAPE;
	case KeyEnter:
		return OIS::KC_NUMPADENTER;
	case KeyReturn:
		return OIS::KC_RETURN;
	case KeyKpAdd:
		return OIS::KC_ADD;
	}

	return OIS::KC_UNASSIGNED;
}

OIS::KeyCode OISWindowsID::mapButton(Button button)
{
	switch (button)
	{
	case ScrollWheelUp:
	case ScrollWheelDown:
		return OIS::KC_SCROLL;
	}

	return OIS::KC_UNASSIGNED;
}