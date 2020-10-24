#pragma once
#include <Windows.h>
#include <Xinput.h>

class Gamepad 
{
public:
	Gamepad(int id);	
	~Gamepad();

	void Update();

	// get if the sticks is inside the deadzone
	bool LStickDead();
	bool RStickDead();

	// get input values of left and right sticks
	float GetLeftStickX();
	float GetLeftStickY();
	float GetRightStickX();
	float GetRightStickY();
	
	// get values of left and right triggers
	float GetLeftTrigger();
	float GetRightTrigger();

	// set rumple amount on left and right motors
	void SetRumble(float leftMotor, float rightMotor);

	// button statuses
	bool IsButtonPressed(WORD button);
	bool WasButtonPressed(WORD button);
	bool WasButtonRealesed(WORD button);

private:

	//store the input states from this and last frame
	XINPUT_STATE _previousState;
	XINPUT_STATE _currentState;	

	// controller port ID
	int _id;	
};
