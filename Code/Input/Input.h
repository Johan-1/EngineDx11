#pragma once
#define DIRECTINPUT_VERSION 0x800
#include <dinput.h>
#include <map>


// define the mouse buttons for read-ability
#define MOUSE0 0
#define MOUSE1 1
#define MOUSE2 2
#define MOUSE3 3

class Input
{
public:
	Input();
	~Input();

	// initialize input
	void InitializeInputDevices(HINSTANCE hinstance, HWND hwnd);

	// updates all devices
	void Update();
		
	// get keyboard buttons
	bool IsKeyHeld(unsigned char key);
	bool IskeyPressed(unsigned char key);
	bool IsKeyReleased(unsigned char key);

	// get mouse buttons
	bool IsMouseHeld(unsigned char button);
	bool IsMousePressed(unsigned char button);
	bool IsMouseReleased(unsigned char button);

	// get mouse movement
	float GetMouseX();
	float GetMouseY();

private:

	// update devices
	void UpdateKeyboard();
	void UpdateMouse();

	// update GUI from input
	void UpdateGuiInput();

	// interface to direct input
	IDirectInput8* _directInput;

	// interface to devices
	IDirectInputDevice8* _keyboard;
	IDirectInputDevice8* _mouse;

	// store states of keyboard this and last frame
	unsigned char _currentkeyboardState[256];
	unsigned char _previouskeyboardState[256];

	// store states of mouse this and last frame
	DIMOUSESTATE _currentMouseState;
	DIMOUSESTATE _previousMouseState;

	// pointer to window handle
	HWND _hwnd;
};


