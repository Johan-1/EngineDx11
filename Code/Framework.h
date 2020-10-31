#pragma once
#include <Windows.h>
#include <string>
#include "SponzaTestScene.h"
#include "IslandTestScene.h"
#include "PBRTestScene.h"

class MasterEditor;
class Window;

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
	MasterEditor* _editor;
	SponzaTestScene* _testScene;
	IslandTestScene* _islandTestScene;
	PBRTestScene* _pbrTest;
};

