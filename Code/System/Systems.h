#pragma once
#include "DXManager.h"
#include "Input.h"
#include "Renderer.h"
#include "GuiManager.h"
#include "LightManager.h"
#include "CameraManager.h"
#include "Time.h"
#include "World.h"
#include "Window.h"
#include "Texturepool.h"

// this class holds the base systems of the engine
// at the moment all of them are static but this might
// change later to use some other dependencies system
class Systems
{
public:
	Systems();
	~Systems();

	static void InitSystems(Window* window, float screenWidth, float screenHeight, bool fullscreen, bool vSync);
	static void DeleteSystems();

	static DXManager*     dxManager;
	static Input*         input;
	static Renderer*      renderer;
	static World*         world;
	static GuiManager*    guiManager;
	static LightManager*  lightManager;
	static CameraManager* cameraManager;
	static Time*          time;
	static TexturePool*   texturePool;
};



