#include "PBRTestScene.h"
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
#include "RotationComponent.h"
#include "ShaderHelpers.h"
#include "PlanarReflectionShader.h"
#include "PostProcessingShader.h"
#include "InstancedModel.h"
#include "MathHelpers.h"
#include "UVScrollComponent.h"
#include "PingPongComponent.h"


PBRTestScene::PBRTestScene()
{
	// get systems
	CameraManager& CM  = *Systems::cameraManager;
	LightManager& LM   = *Systems::lightManager;
	Renderer& renderer = *Systems::renderer;

	// create shadowMap
	Entity* shadowMapRenderer = renderer.CreateShadowMap(200.0f, 8192.0f, XMFLOAT3(-6, 325, 9), XMFLOAT3(85.0f, -90.0f, 0), true);

	// create skybox
	_skyDome = renderer.skyDome = new SkyDome("Data/Settings/SkyDomeCubemap.json");

	renderer.ShowGBufferDebugImages();

	// create game camera
	Entity* cameraGame = new Entity();
	cameraGame->AddComponent<TransformComponent>()->Init(XMFLOAT3(0, 6.22f, 0), XMFLOAT3(13.0f, 89.5f, 0.0f));
	cameraGame->AddComponent<CameraComponent>()->Init3D(90);
	cameraGame->AddComponent<FreeMoveComponent>()->init(12.0f, 0.25f, 6.0f);
	CM.currentCameraGame = cameraGame->GetComponent<CameraComponent>();

	// create UIcamera
	Entity* cameraUI = new Entity();
	cameraUI->AddComponent<TransformComponent>()->Init(XMFLOAT3(0, 0, -1));
	cameraUI->AddComponent<CameraComponent>()->Init2D(XMFLOAT2(SystemSettings::SCREEN_WIDTH, SystemSettings::SCREEN_HEIGHT), XMFLOAT2(0.01f, 10.0f));
	CM.currentCameraUI = cameraUI->GetComponent<CameraComponent>();

	// set ambient light color	
	LM.ambientColor = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	// create directional light and give it pointer to the depth render camera transform
	// it will use the forward of this camera as the light direction
	Entity* directionalLight = new Entity;
	directionalLight->AddComponent<LightDirectionComponent>()->Init(XMFLOAT4(0.8f, 0.8f, 0.8f, 1), shadowMapRenderer->GetComponent<TransformComponent>());

	Entity* floor = new Entity();
	floor->AddComponent<TransformComponent>()->Init(XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(2, 1, 2));
	floor->AddComponent<ModelComponent>()->InitModel("Data/Models/plane.obj", STANDARD | CAST_SHADOW_DIR, L"Data/Textures/PBR/Metal_Rust_Diffuse.dds", L"Data/Textures/PBR/Metal_Rust_Normal.dds", L"Data/Textures/PBR/Metal_Rust_Metalic.dds", L"Data/Textures/PBR/Metal_Rust_Rougness.dds", L"", false, 2);

	Entity* ball1 = new Entity();
	ball1->AddComponent<TransformComponent>()->Init(XMFLOAT3(-3, 1, 3), XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
	ball1->AddComponent<ModelComponent>()->InitModel("Data/Models/sphere.obj", STANDARD | CAST_SHADOW_DIR, L"Data/Textures/PBR/red.dds", L"Data/Textures/PBR/Metal_Plates_Normal.dds", L"", L"", L"");
	ball1->GetComponent<ModelComponent>()->meshes[0]->metalic  = 1.0f;
	ball1->GetComponent<ModelComponent>()->meshes[0]->rougness = 0.0f;

	Entity* ball2 = new Entity();
	ball2->AddComponent<TransformComponent>()->Init(XMFLOAT3(-1, 1, 3), XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
	ball2->AddComponent<ModelComponent>()->InitModel("Data/Models/sphere.obj", STANDARD | CAST_SHADOW_DIR, L"Data/Textures/PBR/red.dds", L"Data/Textures/PBR/Metal_Plates_Normal.dds", L"", L"", L"");
	ball2->GetComponent<ModelComponent>()->meshes[0]->metalic = 1.0f;
	ball2->GetComponent<ModelComponent>()->meshes[0]->rougness = 0.0f;

	Entity* ball3 = new Entity();
	ball3->AddComponent<TransformComponent>()->Init(XMFLOAT3(1, 1, 3), XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
	ball3->AddComponent<ModelComponent>()->InitModel("Data/Models/sphere.obj", STANDARD | CAST_SHADOW_DIR, L"Data/Textures/PBR/red.dds", L"Data/Textures/PBR/Metal_Plates_Normal.dds", L"", L"", L"");
	ball3->GetComponent<ModelComponent>()->meshes[0]->metalic = 1.0f;
	ball3->GetComponent<ModelComponent>()->meshes[0]->rougness = 0.0f;

	Entity* ball4 = new Entity();
	ball4->AddComponent<TransformComponent>()->Init(XMFLOAT3(3, 1, 3), XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
	ball4->AddComponent<ModelComponent>()->InitModel("Data/Models/sphere.obj", STANDARD | CAST_SHADOW_DIR, L"Data/Textures/PBR/red.dds", L"Data/Textures/PBR/Metal_Plates_Normal.dds", L"", L"", L"");
	ball4->GetComponent<ModelComponent>()->meshes[0]->metalic = 1.0f;
	ball4->GetComponent<ModelComponent>()->meshes[0]->rougness = 0.0f;
}

PBRTestScene::~PBRTestScene()
{
}

void PBRTestScene::Update()
{
	const float& delta = Systems::time->GetDeltaTime();
	_skyDome->Update(delta);
}
