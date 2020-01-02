#include "WaterShader.h"
#include "ShaderHelpers.h"
#include "DXManager.h"
#include "DXBlendStates.h"
#include "DXDepthStencilStates.h"
#include "CameraManager.h"
#include "LightManager.h"
#include "Mesh.h"
#include "Systems.h"
#include "RenderToTexture.h"
#include "SystemDefs.h"
#include "Entity.h"
#include "QuadComponent.h"
#include "ParticleShader.h"
#include "SkyDome.h"
#include "SimpleClipSceneShader.h"
#include "MathHelpers.h"
#include "Renderer.h"
#include "DebugQuadHandler.h"
#include "DXRasterizerStates.h"

WaterShader::WaterShader()
{
	//create shaders
	SHADER_HELPERS::CreateVertexShader(L"shaders/vertexForwardWater.shader", _waterVertexShader, waterVertexShaderByteCode);
	SHADER_HELPERS::CreatePixelShader(L"shaders/pixelForwardWater.shader",   _waterPixelShader, waterPixelShaderByteCode);

	// create constant buffers
	SHADER_HELPERS::CreateConstantBuffer(_CBVertex);
	SHADER_HELPERS::CreateConstantBuffer(_CBPixel);

	_simpleClipShaderReflection = new SimpleClipSceneShader(1920, 1080);
	_simpleClipShaderRefraction = new SimpleClipSceneShader(1920, 1080);
}

WaterShader::~WaterShader()
{
	_waterVertexShader->Release();
	_waterPixelShader->Release();

	waterVertexShaderByteCode->Release();
	waterPixelShaderByteCode->Release();

	_CBVertex->Release();

	delete _simpleClipShaderReflection;
	delete _simpleClipShaderRefraction;
}

void WaterShader::ShowDebugQuads()
{
	DebugQuadHandler* quadHandler = Systems::renderer->debugQuadHandler; 
	quadHandler->AddDebugQuad(_simpleClipShaderReflection->GetRenderSRV());
	quadHandler->AddDebugQuad(_simpleClipShaderRefraction->GetRenderSRV());
}

void WaterShader::Render(std::vector<Mesh*>& waterMeshes)
{
	if (waterMeshes.size() == 0)
		return;

	// get systems
	DXManager&     DXM = *Systems::dxManager;
	CameraManager& CM  = *Systems::cameraManager;
	LightManager&  LM  = *Systems::lightManager;
	Renderer& renderer = *Systems::renderer;

	// get device context
	ID3D11DeviceContext*& devCon = DXM.devCon;

	// get game camera and shadow camera
	CameraComponent*& camera      = CM.currentCameraGame;
	CameraComponent*& cameraLight = CM.currentCameraDepthMap;

	TransformComponent* camTrans = camera->GetComponent<TransformComponent>();

	// get camera position
	XMFLOAT3 cameraPos = camTrans->position;
	XMFLOAT3 cameraRot = camTrans->rotation;

	// create constant buffer structures
	CBVertexWater constantVertex;
	XMStoreFloat3(&constantVertex.camPos, XMLoadFloat3(&cameraPos));

	// loop over all meshes that will project reflections onto itself
	size_t numMeshes = waterMeshes.size();
	for (int i = 0; i < numMeshes; i++)
	{
		// get the mesh that will project the reflections
		Mesh* mesh = waterMeshes[i];

		// get the clip plane of the water mesh so we dont render anything above this plane
		XMFLOAT4 clipPlaneRefract(0, -1, 0, mesh->GetPosition().y);

		// render the refractionmap
		_simpleClipShaderRefraction->RenderScene(renderer.GetMeshes(SHADER_TYPE::S_REFRACT_OPAQUE), renderer.GetMeshes(SHADER_TYPE::S_REFRACT_ALPHA), renderer.GetInstancedModels(INSTANCED_SHADER_TYPE::S_INSTANCED_REFRACT), clipPlaneRefract, true, false);

		// get the clip plane of the water mesh so we dont render anything below this plane
		XMFLOAT4 clipPlaneReflect(0, 1, 0, -mesh->GetPosition().y);

		// change the camera y position to render reflection
		XMFLOAT3 reflectPos = cameraPos;
		reflectPos.y -= (cameraPos.y - mesh->GetPosition().y) * 2.0f;

		// set reflection position/rotation and build matrices
		camTrans->rotation = XMFLOAT3(-cameraRot.x, cameraRot.y, cameraRot.z);
		camTrans->position = reflectPos;
		camTrans->BuildWorldMatrix();
		camera->CalculateViewMatrix();

		// save the reflection viewprojmatrix
		XMFLOAT4X4 reflectMat = camera->viewProjMatrix;

		// render the reflectionmap
		_simpleClipShaderReflection->RenderScene(renderer.GetMeshes(SHADER_TYPE::S_CAST_REFLECTION_OPAQUE), renderer.GetMeshes(SHADER_TYPE::S_CAST_REFLECTION_ALPHA), renderer.GetInstancedModels(INSTANCED_SHADER_TYPE::S_INSTANCED_CAST_REFLECTION), clipPlaneReflect, true);

		// change back to original camera position/rotation
		camTrans->rotation = cameraRot;
		camTrans->position = cameraPos;
		camTrans->BuildWorldMatrix();
		camera->CalculateViewMatrix();

		// set back to defualt render target and render the reflection mesh
		Systems::renderer->SetMainRenderTarget();

		// set shaders
		devCon->VSSetShader(_waterVertexShader, NULL, 0);
		devCon->PSSetShader(_waterPixelShader, NULL, 0);

		// set the vertex constant buffer, the pixel ones is already set
		devCon->VSSetConstantBuffers(0, 1, &_CBVertex);
		devCon->PSSetConstantBuffers(0, 1, &LM.cbAmbDir);
		devCon->PSSetConstantBuffers(1, 1, &LM.cbPoint);
		devCon->PSSetConstantBuffers(2, 1, &_CBPixel);

		// set to alpha blending
		DXM.blendStates->SetBlendState(BLEND_STATE::BLEND_ALPHA);

		// set vertex constant values
		const XMFLOAT4X4& worldMat = mesh->GetWorldMatrix();

		XMStoreFloat4x4(&constantVertex.world,                XMLoadFloat4x4(&mesh->GetWorldMatrixTrans()));
		XMStoreFloat4x4(&constantVertex.worldViewProj,        XMLoadFloat4x4(&MATH_HELPERS::MatrixMutiplyTrans(&worldMat, &camera->viewProjMatrix)));
		XMStoreFloat4x4(&constantVertex.worldViewProjLight,   XMLoadFloat4x4(&MATH_HELPERS::MatrixMutiplyTrans(&worldMat, &cameraLight->viewProjMatrix)));
		XMStoreFloat4x4(&constantVertex.worldViewProjReflect, XMLoadFloat4x4(&MATH_HELPERS::MatrixMutiplyTrans(&worldMat, &reflectMat)));
		XMStoreFloat2(&constantVertex.uvOffset,               XMLoadFloat2(&mesh->uvOffset));

		// update vertex constant buffer
		SHADER_HELPERS::UpdateConstantBuffer((void*)&constantVertex,      sizeof(CBVertexWater), _CBVertex);
		SHADER_HELPERS::UpdateConstantBuffer((void*)&mesh->waterSettings, sizeof(WaterSettings), _CBPixel);

		// fill texture array with all textures including the shadow map and reflection map
		ID3D11ShaderResourceView* t[10] = 
		{
			mesh->baseTextures[1],
			mesh->baseTextures[2],
			mesh->DUDVMap, 
			cameraLight->renderTexture, 
			_simpleClipShaderReflection->GetRenderSRV(), 
			_simpleClipShaderRefraction->GetRenderSRV(), 
			_simpleClipShaderRefraction->GetDepthSRV(),
			mesh->foamMap,
			mesh->noiseMap,
			mesh->normalMap2
		};

		// set SRV's
		devCon->PSSetShaderResources(0, 10, t);

		mesh->UploadBuffers();

		DXM.rasterizerStates->SetRasterizerState(RASTERIZER_STATE::NOCULL);

		devCon->DrawIndexed(mesh->numIndices, 0, 0);

		DXM.rasterizerStates->SetRasterizerState(RASTERIZER_STATE::BACKCULL);

		// unbind so we can use resources as input in next stages
		ID3D11ShaderResourceView* nullSRV[10] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
		devCon->PSSetShaderResources(0, 10, nullSRV);
	}
}
