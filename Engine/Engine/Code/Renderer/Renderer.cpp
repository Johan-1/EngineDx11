#include "Renderer.h"
#include "DXManager.h"
#include "TransformComponent.h"
#include "CameraComponent.h"
#include "CameraManager.h"
#include "Entity.h"
#include "RenderToTexture.h"
#include "QuadComponent.h"
#include "SkyDome.h"
#include "Mesh.h"
#include "SystemDefs.h"
#include <algorithm>
#include "GBuffer.h"
#include "ScreenQuad.h"
#include "DepthShader.h"
#include "DeferredShader.h"
#include "QuadShader.h"
#include "ParticleShader.h"
#include "ImGUIShader.h"
#include "ForwardAlphaShader.h"
#include "DXRasterizerStates.h"
#include "WireframeShader.h"
#include "Input.h"
#include "Systems.h"
#include "FreeMoveComponent.h"
#include "PlanarReflectionShader.h"
#include "PostProcessingShader.h"
#include "SimpleClipSceneShader.h"
#include "WaterShader.h"
#include "DebugQuadHandler.h"

using namespace DirectX;

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
	delete _depthMap;
	delete _gBuffer;
	delete fullScreenQuad;
	delete skyDome;
	delete depthShader;
	delete deferredShader;
	delete quadShader;
	delete particleShader;
	delete imGUIShader;
	delete forwardAlphaShader;
	delete inputLayouts;
	delete planarReflectionShader;
	delete postProcessingShader;
	delete waterShader;
	delete debugQuadHandler;
}

void Renderer::Initialize()
{
	// system for handeling creating and displaying
	// debug UI quads of render targets
	debugQuadHandler = new DebugQuadHandler();

	// set clear color
	SetClearColor(0, 0, 0, 1);

	// create and compile shaders
	depthShader            = new DepthShader();
	deferredShader         = new DeferredShader();
	quadShader             = new QuadShader();
	particleShader         = new ParticleShader();
	imGUIShader            = new ImGUIShader();
	forwardAlphaShader     = new ForwardAlphaShader();
	wireframeShader        = new WireframeShader();
	planarReflectionShader = new PlanarReflectionShader();
	postProcessingShader   = new PostProcessingShader();
	waterShader            = new WaterShader();

	// create input layouts
	inputLayouts = new DXInputLayouts();
	inputLayouts->CreateInputLayout3D(deferredShader->vertexGeometryShaderByteCode);
	inputLayouts->CreateInputLayout3DInstanced(deferredShader->vertexGeometryShaderByteCodeInstanced);
	inputLayouts->CreateInputLayout2D(quadShader->vertexShaderByteCode);
	inputLayouts->CreateInputLayoutParticle(particleShader->vertexShaderByteCode);
	inputLayouts->CreateInputLayoutGUI(imGUIShader->GetVertexShaderByteCode());

	// create gbuffer for deffered rendering
	_gBuffer = new GBuffer();

	// create fullscreenquad for deferred rendering 
	// and to project the final scene image after post processing
	fullScreenQuad = new ScreenQuad();

	// create the main rendertarget we will use
	// this will hold our final scene up till we apply post processing
	// and render to the backbuffer
	mainRendertarget = new RenderToTexture((unsigned int)SystemSettings::SCREEN_WIDTH, (unsigned int)SystemSettings::SCREEN_HEIGHT, false, SystemSettings::USE_HDR, false);
}

Entity* Renderer::CreateShadowMap(float orthoSize, float resolution, XMFLOAT3 position, XMFLOAT3 rotation, bool debugQuad)
{
	// create depthmap render texture
	_depthMap = new RenderToTexture((unsigned int)resolution, (unsigned int)resolution, true, false, false, debugQuad);

	// create camera entity with orthographic view for shadowmap rendering
	_cameraDepth = new Entity();
	_cameraDepth->AddComponent<TransformComponent>()->Init(position, rotation);
	_cameraDepth->AddComponent<CameraComponent>()->Init2D(XMFLOAT2(orthoSize, orthoSize), XMFLOAT2(0.01f, 5000.0f));
	_cameraDepth->AddComponent<FreeMoveComponent>()->init(80, 0.1f);

	// start the free move component of shadow camera inactive
	_cameraDepth->GetComponent<FreeMoveComponent>()->SetActive(false);

	// get pointer to camera component
	CameraComponent* depthCamera = _cameraDepth->GetComponent<CameraComponent>();

	// give camera a reference to the SRV in depthMap render texture
	depthCamera->renderTexture = _depthMap->depthStencilSRV;

	// set this camera to the active depth render camera
	Systems::cameraManager->currentCameraDepthMap = depthCamera;

	return _cameraDepth;
}

void Renderer::ShowGBufferDebugImages()
{
	debugQuadHandler->AddDebugQuad(_gBuffer->SRVArray[0]);
	debugQuadHandler->AddDebugQuad(_gBuffer->SRVArray[1]);
	debugQuadHandler->AddDebugQuad(_gBuffer->SRVArray[2]);
	debugQuadHandler->AddDebugQuad(_gBuffer->SRVArray[3]);
}

void Renderer::Render() 
{	
	// get dx manager
	DXManager& dXM = *Systems::dxManager;

	// update the light buffers for GPU input
	Systems::lightManager->UpdateLightBuffers();

	// clear the backbuffer and main rendertarget buffers
	dXM.ClearRenderTarget(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
	mainRendertarget->ClearRenderTarget(clearColor[0], clearColor[1], clearColor[2], clearColor[3], false);
	
	// render shadowmap
	RenderDepth();

	// render all geometry and then fullscreen quad with lightcalculations
	RenderDeferred();		

	// render skybox, will mask out all pixels that contains geometry in the fullscreen quad, leaving only the skybox rendered on "empty" pixels
	inputLayouts->SetInputLayout(INPUT_LAYOUT_TYPE::LAYOUT_3D);
	skyDome->Render();

	// render debug wireframe meshes, these are forward rendered
	wireframeShader->RenderWireFrame(_meshes[S_WIREFRAME]);

	// render planar reflections forward
	planarReflectionShader->Render(_meshes[S_ALPHA_REFLECTION]);

	// render reflective/refractive water
	waterShader->Render(_meshes[S_ALPHA_WATER]);

	// render alpha meshes
	forwardAlphaShader->RenderForward(_meshes[S_FORWARD_ALPHA]);

	// render particles
	inputLayouts->SetInputLayout(INPUT_LAYOUT_TYPE::LAYOUT_PARTICLE);
	particleShader->RenderParticles(_particleSystems);

	// render the final 2d stuff 
	inputLayouts->SetInputLayout(INPUT_LAYOUT_TYPE::LAYOUT_2D);

	// Render post processing 
	postProcessingShader->Render(fullScreenQuad, mainRendertarget->renderTargetSRV, mainRendertarget->depthStencilSRV);

	// render UI
	quadShader->RenderQuadUI(_quads);
	
	// render IM GUI
	inputLayouts->SetInputLayout(INPUT_LAYOUT_TYPE::LAYOUT_GUI);
	imGUIShader->RenderGUI();
}

void Renderer::RenderDeferred()
{
	DXManager& dXM = *Systems::dxManager;
		
	// set the rendertargets of the GBuffer active		
	_gBuffer->SetRenderTargets(mainRendertarget->depthStencilView);		

	// render all geometry info to the render targets
	inputLayouts->SetInputLayout(INPUT_LAYOUT_TYPE::LAYOUT_3D);
	deferredShader->RenderGeometry(_meshes[S_DEFERRED]);

	inputLayouts->SetInputLayout(INPUT_LAYOUT_TYPE::LAYOUT_3D_INSTANCED);
	deferredShader->renderGeometryInstanced(_instancedModels[S_INSTANCED_DEFERRED]);

	// set to 2D input Layout
	inputLayouts->SetInputLayout(INPUT_LAYOUT_TYPE::LAYOUT_2D);

	// set to defualt rendertarget 
	mainRendertarget->SetRendertarget(false, false);

	// upload the vertices of the screensized quad
	fullScreenQuad->UploadBuffers();

	// render lights as 2d post processing
	deferredShader->RenderLightning(_gBuffer);
}

void Renderer::RenderDepth() 
{
	if (_meshes[S_DEPTH].size() == 0)
		return;

	// get the DX manager
	DXManager& dXM = *Systems::dxManager;

	// clear the depth map render texture to black
	_depthMap->ClearRenderTarget(0, 0, 0, 0, true);

	_depthMap->SetRendertarget(true, false);

	// render all meshes 
	inputLayouts->SetInputLayout(INPUT_LAYOUT_TYPE::LAYOUT_3D);
	depthShader->RenderDepth(_meshes[S_DEPTH]);

	inputLayouts->SetInputLayout(INPUT_LAYOUT_TYPE::LAYOUT_3D_INSTANCED);
	depthShader->RenderDepthInstanced(_instancedModels[S_INSTANCED_DEPTH]);
}

void Renderer::SetMainRenderTarget()
{
	mainRendertarget->SetRendertarget(false, false); 
}