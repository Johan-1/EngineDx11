#pragma once
#include <Windows.h>

class GameTime
{
public:
	GameTime();
	~GameTime();

	// update time and handle when window changes
	void Update();
	void OnWindowChange();	
	
	// get delta and fps
	const float& GetDeltaTime() { return _delta; }
	const int&   GetFps()       { return _fps; }

private:

	// calculate delta and fps
	void CalculateDelta();
	void CalculateFPS();

	// for delta
	float _ticksPerSecond;	
	INT64 _startTimeDelta;
	float _delta;

	// save last frames delta
	bool  _useLastDelta;
	float _lastDelta;

	// for fps
	int _fps,_count;
	unsigned long _startTimeFps;


};

