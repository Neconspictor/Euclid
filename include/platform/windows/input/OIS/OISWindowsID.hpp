#ifndef OIS_WINDOWS_ID
#define OIS_WINDOWS_ID
#include "input/Input.hpp"
#include <windows.h>
#include <OIS/OIS.h>

class OISWindowsID : public Input
{
public:

	OISWindowsID(HWND windows, int width, int height);
	virtual ~OISWindowsID();
	bool isDown(Key key) override;
	bool isDown(Button button) override;
	bool isPressed(Key key) override;
	bool isPressed(Button button) override;
	bool isReleased(Key key) override;
	bool isReleased(Button button) override;
	Key getAnyPressedKey() override;
	Button getAnyPressedButton() override;

	void update();

private:

	OIS::InputManager* m_InputManager;
	OIS::Mouse* m_Mouse;
	OIS::Keyboard* m_Keyboard;

	KeyState pressedKeys[KEY_SIZE];

	OIS::KeyCode mapKey(Key key);
	OIS::KeyCode mapButton(Button button);
};
#endif