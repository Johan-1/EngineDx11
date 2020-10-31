#include "IslandTestScene.h"
#include "DXManager.h"
#include "CameraManager.h"
#include "LightManager.h"
#include "Renderer.h"
#include "Systems.h"
#include "SystemDefs.h"
#include "Mesh.h"
#include "ModelComponent.h"
#include "ParticleSystemComponent.h"
#include "LightPointComponent.h"
#include "CameraComponent.h"
#include "FreeMoveComponent.h"
#include "UVScrollComponent.h"
#include "WaterShader.h"
#include "PostProcessingShader.h"
#include "InstancedModel.h"
#include "MathHelpers.h"

IslandTestScene::IslandTestScene()
{
	//// get systems
	//CameraManager& CM  = *Systems::cameraManager;
	//LightManager& LM   = *Systems::lightManager;
	//Renderer& renderer = *Systems::renderer;

	//// create shadowMap
	//Entity* shadowMapRenderer = renderer.CreateShadowMap(100.0f, 8192.0f, XMFLOAT3(-6, 325, 9), XMFLOAT3(85.0f, -90.0f, 0), true);

	//// create skybox
	//_skyDome = renderer.skyDome = new SkyDome("Settings/SkyDomeDefault.json");

	//renderer.ShowGBufferDebugImages();
	//renderer.waterShader->ShowDebugQuads();
	//renderer.postProcessingShader->ShowBloomBlurP2DebugQuad();

	//// create game camera
	//Entity* cameraGame = new Entity();
	//cameraGame->AddComponent<TransformComponent>()->Init(XMFLOAT3(-64.28f, 12.22f, 0.41f), XMFLOAT3(13.0f, 89.5f, 0.0f));
	//cameraGame->AddComponent<CameraComponent>()->Init3D(90);
	//cameraGame->AddComponent<FreeMoveComponent>()->init(12.0f, 0.25f, 4.0f);
	//CM.currentCameraGame = cameraGame->GetComponent<CameraComponent>();

	//// create UIcamera
	//Entity* cameraUI = new Entity();
	//cameraUI->AddComponent<TransformComponent>()->Init(XMFLOAT3(0, 0, -1));
	//cameraUI->AddComponent<CameraComponent>()->Init2D(XMFLOAT2(SystemSettings::SCREEN_WIDTH, SystemSettings::SCREEN_HEIGHT), XMFLOAT2(0.01f, 10.0f));
	//CM.currentCameraUI = cameraUI->GetComponent<CameraComponent>();

	//// set ambient light color	
	//LM.ambientColor = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	//// create directional light and give it pointer to the depth render camera transform
	//// it will use the forward of this camera as the light direction
	//Entity* directionalLight = new Entity;
	//directionalLight->AddComponent<LightDirectionComponent>()->Init(XMFLOAT4(0.8f, 0.8f, 0.8f, 1), shadowMapRenderer->GetComponent<TransformComponent>());

	//Entity* lake = new Entity();
	//lake->AddComponent<TransformComponent>()->Init(XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(400.0f, 1.0f, 400.0f));
	//lake->AddComponent<ModelComponent>()->InitModel("models/plane.obj", STANDARD | REFRACT, L"Textures/sand.dds", L"Textures/sandNormal.dds", L"Textures/sandSpecular.dds", L"", false, 800);

	//Entity* water = new Entity();
	//water->AddComponent<TransformComponent>()->Init(XMFLOAT3(0, 2.0f, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(400, 1, 400));
	//water->AddComponent<ModelComponent>()->InitModel("models/plane.obj", WATER, L"", L"Textures/waterNormal.dds", L"Textures/FlatHighSpecular.dds", L"", false, 400.0f);
	//water->AddComponent<UVScrollComponent>()->Init(XMFLOAT2(0.015f, -0.01f));
	//water->GetComponent<ModelComponent>()->SetDUDVMap(L"Textures/waterDUDV.dds");
	//water->GetComponent<ModelComponent>()->SetFoamMap(L"Textures/foam3.dds");
	//water->GetComponent<ModelComponent>()->SetNoiseMap(L"Textures/perlinNoise2.dds");

	//Entity* tree = new Entity();
	//tree->AddComponent<TransformComponent>()->Init(XMFLOAT3(0, 0.0f, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(0.05f, 0.05f, 0.05f));
	//tree->AddComponent<ModelComponent>()->InitModel("Models/white_oak.obj", STANDARD | CAST_REFLECTION | CAST_SHADOW_DIR | REFRACT);

	//Entity* sphere = new Entity();
	//sphere->AddComponent<TransformComponent>()->Init(XMFLOAT3(0, 4, 3), XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
	//sphere->AddComponent<ModelComponent>()->InitModel("Models/Sphere.obj", STANDARD | CAST_SHADOW_DIR | CAST_REFLECTION, L"", L"", L"", L"Textures/emissiveTest.dds", false);
	//sphere->AddComponent<LightPointComponent>()->Init(5, 5, XMFLOAT3(0.1f, 1.0f, 0), 0.0f, 1.0f, 0.2f);

	//Entity* tree1 = new Entity();
	//tree1->AddComponent<TransformComponent>()->Init(XMFLOAT3(0, 2.0f, 30), XMFLOAT3(0, 0, 0), XMFLOAT3(0.2f, 0.2f, 0.2f));
	//tree1->AddComponent<ModelComponent>()->InitModel("Models/Palm_01.obj", STANDARD | CAST_REFLECTION | CAST_SHADOW_DIR);
}

IslandTestScene::~IslandTestScene()
{
}

void IslandTestScene::Update(const float& delta)
{
	//_skyDome->Update(delta);
}