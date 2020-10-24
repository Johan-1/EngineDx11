#include "Gamepad.h"

Gamepad::Gamepad(int id)
{	
	// initialize memory of input states
	ZeroMemory(&_previousState, sizeof(XINPUT_STATE));
	ZeroMemory(&_currentState,  sizeof(XINPUT_STATE));		
	_id = id;
}

Gamepad::~Gamepad()
{
}

void Gamepad::Update() 
{		
	// get the state of this frame
	XINPUT_STATE state;
	if (XInputGetState(_id, &state) == ERROR_SUCCESS)
	{
		// set the previous state before we update our current state
		_previousState = _currentState;
		_currentState = state;			
	}					
}

// check if the left stick is in the defualt deadzone
bool Gamepad::LStickDead() 
{
	short X = _currentState.Gamepad.sThumbLX;
	short Y = _currentState.Gamepad.sThumbLY;

	if (X > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || X < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		return false;

	if (Y > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || Y < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		return false;

	return true;
}

// check if the right stick is in the defualt deadzone
bool Gamepad::RStickDead() 
{
	short X = _currentState.Gamepad.sThumbRX;
	short Y = _currentState.Gamepad.sThumbRY;

	if (X > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || X < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		return false;

	if (Y > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || Y < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		return false;

	return true;
}

// return the leftstick X value between -1 to 1
float Gamepad::GetLeftStickX() 
{
	short X = _currentState.Gamepad.sThumbLX;

	return (static_cast <float> (X) / 32768.0f);
}

// return the leftstick Y value between -1 to 1
float Gamepad::GetLeftStickY() 
{
	short Y = _currentState.Gamepad.sThumbLY;
	
	return (static_cast <float> (Y) / 32768.0f);
}

// return the rightstick X value between -1 to 1
float Gamepad::GetRightStickX() 
{
	short X = _currentState.Gamepad.sThumbRX;
	
	return (static_cast <float> (X) / 32768.0f);
}

// return the rightstick Y value between -1 to 1
float Gamepad::GetRightStickY() 
{
	short Y = _currentState.Gamepad.sThumbRY;

	return (static_cast <float> (Y) / 32768.0f);
}

// return the normalized left trigger value
float Gamepad::GetLeftTrigger() 
{
	BYTE trigger = _currentState.Gamepad.bLeftTrigger;

	return (static_cast <float> (trigger) / 255.0f);
}

// return the normalized right trigger value
float Gamepad::GetRightTrigger() 
{
	BYTE trigger = _currentState.Gamepad.bRightTrigger;

	return (static_cast <float> (trigger) / 255.0f);
}

// set the power of the vibration motors from a normalized value
void Gamepad::SetRumble(float leftMotor, float rightMotor) 
{
	XINPUT_VIBRATION state;

	if      (leftMotor < 0) leftMotor = 0;
	else if (leftMotor > 1) leftMotor = 1;

	if      (rightMotor < 0) rightMotor = 0;
	else if (rightMotor > 1) rightMotor = 1;

	ZeroMemory(&state, sizeof(XINPUT_VIBRATION));

	float left  = leftMotor  * 65535.0f;
	float right = rightMotor * 65535.0f;

	state.wLeftMotorSpeed  = left;
	state.wRightMotorSpeed = right;

	XInputSetState(_id, &state);
}

// checks if a button is pressed down
bool Gamepad::IsButtonPressed(WORD button)
{
	return ((_currentState.Gamepad.wButtons & button) != 0);
}

// checks if a button was just pressed this frame
bool Gamepad::WasButtonPressed(WORD button) 
{
	return (((_previousState.Gamepad.wButtons & button) == 0) && ((_currentState.Gamepad.wButtons & button) != 0));
}

// checks if a button was released this frame
bool Gamepad::WasButtonRealesed(WORD button)
{
	return (((_previousState.Gamepad.wButtons & button) != 0) && ((_currentState.Gamepad.wButtons & button) == 0));
}
