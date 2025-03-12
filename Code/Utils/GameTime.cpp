#include "GameTime.h"
#include <iostream>
#include <mmsystem.h>
#include "SystemDefs.h"

GameTime::GameTime() :
	_ticksPerSecond(0),	
	_startTimeDelta(0),
	_delta(0),
	_startTimeFps(0),
	_count(0),
	_fps(0),
	_lastDelta(0)
{
}

GameTime::~GameTime()
{
}

void GameTime::OnWindowChange()
{
	_useLastDelta = true;
	_lastDelta    = _delta;
}

void GameTime::Update()
{
	CalculateDelta();
	CalculateFPS();
}

void GameTime::CalculateDelta()
{
	// if this is the first frame of the application
	// setup the start delta
	if (_delta == 0)
	{
		INT64 frequancy = 0;
		// returns the number of ticks per second
		QueryPerformanceFrequency((LARGE_INTEGER*)&frequancy);
		if (frequancy == 0)
			printf("System do not support high performance timers\n");

		// convert to ticks per ms
		_ticksPerSecond = (float)frequancy;

		// get number of ticks elapsed before we start time calculations
		QueryPerformanceCounter((LARGE_INTEGER*)&_startTimeDelta);

		// get time elapsed in ms
		_startTimeFps = timeGetTime();
	}

	INT64 currentTime = 0;

	// get the current time
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	// get number of ticks from last frame to this frame
	INT64 difference = currentTime - _startTimeDelta;

	// divide elapsed ticks with tickrate per second 
	_delta = difference / _ticksPerSecond;

	if (_useLastDelta)
	{
		_delta = _lastDelta;
		_useLastDelta = false;
	}

	// set start time for next frame;
	_startTimeDelta = currentTime;

}

void GameTime::CalculateFPS()
{
	// count the frames and when one second have passed set fps to number of frames and reset count and start time
	_count++;
	if (timeGetTime() >= (_startTimeFps + 1000)) 
	{
		_fps = _count;

		_startTimeFps = timeGetTime();
		_count = 0;
	}
}