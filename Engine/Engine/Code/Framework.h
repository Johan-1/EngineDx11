#pragma once
#include <Windows.h>
#include <string>

class DebugStats;
class Window;
class Entity;

class Framework
{
public:
	Framework();
	~Framework();

private:

	// functions that make up our application loop
	void Start();
	void Run();
	void Update();
	void Render();

	// helper for generating a random float
	float GetRandomFloat(float min, float max);

	// window and debug stats
	Window*     _window;
	DebugStats* _debugStats;

	Entity* _ent;
};
