#include "input.h"
#include <iostream>
#include "DXManager.h"
#include "imgui.h"

Input::Input() :
	_directInput(nullptr),
	_keyboard(nullptr),
	_mouse(nullptr)
{	
}

Input::~Input()
{
	_keyboard->Unacquire();
	_keyboard->Release();

	_mouse->Unacquire();
	_mouse->Release();

	_directInput->Release();
}

void Input::InitializeInputDevices(HINSTANCE hinstance, HWND hwnd) 
{
	HRESULT result;
	_hwnd = hwnd;

	// create directinput interface
	result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&_directInput, NULL);
	if (FAILED(result)) printf("failed to create directInput interface\n");

	// create keyboard device 
	result = _directInput->CreateDevice(GUID_SysKeyboard, &_keyboard, NULL);
	if (FAILED(result)) printf("failed to create keyboard interface\n");

	// set keyboard data format to defualt
	result = _keyboard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(result)) printf("failed to set data format for keyboard\n");

	// set application to exclusivly receive input when in main focus
	result = _keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if (FAILED(result)) printf("failed to set cooperative level for keyboard\n");
	
	// aquire acces to keyboard
	result = _keyboard->Acquire();
	if (FAILED(result)) printf("failed to get acces over keyboard\n");

	// create mouse device
	result = _directInput->CreateDevice(GUID_SysMouse, &_mouse, NULL);
	if (FAILED(result)) printf("failed to create mouse interface\n");

	// set mouse data format to default
	result = _mouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(result)) printf("failed to set data format for mouise\n");

	// mouse can be used by other applications even when this window is in foreground
	result = _mouse->SetCooperativeLevel(hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(result)) printf("failed to set cooperative level for mouse\n");

	// aquire acces to keyboard
	result = _mouse->Acquire();
	if (FAILED(result)) printf("failed to get acces over mouse\n");	
}

void Input::Update() 
{
	UpdateKeyboard();
	UpdateMouse();
	UpdateGuiInput();
}

// get the current state of the keyboard
void Input::UpdateKeyboard() 
{
	HRESULT result;	
	// copy last frames current input data to previous state
	std::copy(_currentkeyboardState, _currentkeyboardState + 256, _previouskeyboardState);
	
	result = _keyboard->GetDeviceState(sizeof(_currentkeyboardState), (LPVOID)&_currentkeyboardState);
	if (FAILED(result)) 
	{
		// try to reaquire
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))		
			_keyboard->Acquire();
		else
			printf("failed to get keyboardstate\n");		
	}
}

// get the current state of the mouse
void Input::UpdateMouse() 
{
	HRESULT result;

	_previousMouseState = _currentMouseState;

	result = _mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&_currentMouseState);
	if (FAILED(result))
	{
		// try to reaquire
		if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
			_keyboard->Acquire();
		else
			printf("failed to get mousestate\n");
	}
}

void Input::UpdateGuiInput()
{
	ImGuiIO& io = ImGui::GetIO();

	// get pos of mousecursor
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(_hwnd, &p);

	// give imgui the coords
	io.MousePos.x = p.x;
	io.MousePos.y = p.y;

	// set mousebuttons
	for (int i = 0; i < 4; i++)			
		io.MouseDown[i] = _currentMouseState.rgbButtons[i] & 0x80;
			
	//obtain keyboard information
	HKL layout = GetKeyboardLayout(0);
	UCHAR keyboardState[256];
	if (GetKeyboardState(keyboardState) == false)
		return;

	for (int i = 0; i < 256; i++)
	{
		// set keys down
		io.KeysDown[i] = _currentkeyboardState[i] & 0x80;

		// if button just pressed ad character of key to imgui
		if ((_currentkeyboardState[i] & 0x80) != 0 && (_previouskeyboardState[i] & 0x80) == 0)
		{
			if (IsKeyHeld(DIK_LSHIFT))
				keyboardState[0x10] = 0x80; // set bit for shift to true
											// get key from layout
			UINT vk = MapVirtualKeyEx(i, 1, layout);

			// convert to ascii character
			USHORT asciiValue;
			ToAscii(vk, i, keyboardState, &asciiValue, 0);
			char c = (char)asciiValue;

			if (c > 0)
				io.AddInputCharacter((unsigned short)c);
		}
	}						
}

// is a key down
bool Input::IsKeyHeld(unsigned char key) 
{
	return (_currentkeyboardState[key] & 0x80);	
}

// was a key pressed this frame
bool Input::IskeyPressed(unsigned char key) 
{		
	return (_currentkeyboardState[key] & 0x80) != 0 && (_previouskeyboardState[key] & 0x80) == 0;
}

// was a key released this frame
bool Input::IsKeyReleased(unsigned char key) 
{	
	return (_currentkeyboardState[key] & 0x80) == 0 && (_previouskeyboardState[key] & 0x80) != 0;
}

// get mouse X movement of this frame
float Input::GetMouseX() 
{
	return (float)_currentMouseState.lX;	
}

// get mouse y movement of this frame
float Input::GetMouseY() 
{
	return (float)_currentMouseState.lY;
}

// is a mouse button down
bool Input::IsMouseHeld(unsigned char button)
{
	return _currentMouseState.rgbButtons[button] & 0x80; 
}

// was a mouse button pressed this frame
bool Input::IsMousePressed(unsigned char button)
{
	return (_currentMouseState.rgbButtons[button] & 0x80) != 0 && (_previousMouseState.rgbButtons[button] & 0x80) == 0;
}

// was a mouse button released this frame
bool Input::IsMouseReleased(unsigned char button)
{
	return (_currentMouseState.rgbButtons[button] & 0x80) == 0 && (_previousMouseState.rgbButtons[button] & 0x80) != 0;
}

