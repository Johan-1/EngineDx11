#include "Framework.h"
#include <d3d11.h>
#include <iostream>
#include <DirectXMath.h>
#include "Framework.h"
#include "SystemDefs.h"
#include "DXManager.h"
#include "CameraManager.h"
#include "LightManager.h"
#include "Renderer.h"
#include "World.h"
#include "input.h"
#include "Time.h"
#include "Mesh.h"
#include "Entity.h"
#include "FreeMoveComponent.h"
#include "RotationComponent.h"
#include "UVScrollComponent.h"
#include "CameraComponent.h"
#include "QuadComponent.h"
#include "ModelComponent.h"
#include "ParticleSystemComponent.h"
#include "LightPointComponent.h"
#include <iostream>
#include "GuiManager.h"
#include "MasterEditor.h"
#include <algorithm>
#include <random>
#include <chrono>
#include "Color32.h"
#include "Window.h"
#include "Systems.h"
#include "SponzaTestScene.h"
#include "IslandTestScene.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM Lparam);

Framework::Framework()
{
	PostProcessing::ReadSettings("Data/Settings/PostProcessDefualt.json");
	SystemSettings::ReadSettings("Data/Settings/SystemDefault.json");

	// create the window for the application
	_window = new Window("Engine", 0, 0, (unsigned int)SystemSettings::SCREEN_WIDTH, (unsigned int)SystemSettings::SCREEN_HEIGHT, WndProc);
	
	// init all systems
	Systems::InitSystems(_window, SystemSettings::SCREEN_WIDTH, SystemSettings::SCREEN_HEIGHT, SystemSettings::FULLSCREEN, SystemSettings::V_SYNC);

	// start and run
	Start();
	Run();	
}

Framework::~Framework()
{
	delete _window;
	Systems::DeleteSystems();
}

void Framework::Start()
{
	//_islandTestScene = new IslandTestScene();
	_testScene         = new SponzaTestScene();
	//_pbrTest = new PBRTestScene();
	_editor  = new MasterEditor();
}

void Framework::Update()
{
	Systems::world->Update();
	//_islandTestScene->Update(Systems::time->GetDeltaTime());
	_testScene->Update();
	//_pbrTest->Update();
	_editor->Update();
}

void Framework::Render()
{
	Systems::renderer->Render();
}

void Framework::Run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		DXManager& DXM = *Systems::dxManager;

		// update imgui
		Systems::guiManager->Update();

		// update timers
		Systems::time->Update();

		// uppdate input
		Systems::input->Update();
		
		// destroy window if escape is pressed, will send quit message with windowproc to end loop
		if (Systems::input->IskeyPressed(DIK_ESCAPE))
		{
			DXM.SetFullscreen(false);
			DestroyWindow(_window->hwnd);
		}

		// update everything
		Update();
		
		// render everything
		Render();

		// swap buffers
		DXM.PresentScene();
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM Lparam)
{
	// just handle closing, destroying and quit messages with wndproc, all other input is done with directinput
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		SetFocus(GetConsoleWindow());
		break;
	case WM_SETCURSOR:
		Systems::guiManager->UpdateMouseCursor();
		break;
		// sent when size,position or Z order is changed
		// works for everything that is freezing the application
		// like draging window, fullscreen change and using any windows type functions 
		// like opening the file directory to save/open files
		// when this happens we need to save the last deltaTime
		// so we can use it for the first frame when the application starts uppdating again
		// otherwise we end up with a massive delta during the first frame
	case WM_WINDOWPOSCHANGING:
		if (Systems::time != nullptr)
			Systems::time->OnWindowChange();
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, Lparam);
	}

	return 0;
}

// returns a randomized float
float Framework::GetRandomFloat(float min, float max)
{
	unsigned int seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count();
	std::mt19937 generator(seed);
	std::uniform_real_distribution<float> distribution(min, max);

	return distribution(generator);
}