#include "PlanarReflectionShader.h"
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

PlanarReflectionShader::PlanarReflectionShader()
{
	//create shaders
	SHADER_HELPERS::CreateVertexShader(L"shaders/vertexForwardPlanar.shader", _planarVertexShader, planarVertexShaderByteCode);
	SHADER_HELPERS::CreatePixelShader(L"shaders/pixelForwardPlanar.shader",   _planarPixelShader, planarPixelShaderByteCode);

	// create constant buffers
	SHADER_HELPERS::CreateConstantBuffer(_CBVertex);
	SHADER_HELPERS::CreateConstantBuffer(_CBReflect);

	_simpleClipShaderReflection = new SimpleClipSceneShader(1920, 1080);
}

PlanarReflectionShader::~PlanarReflectionShader()
{
	_planarVertexShader->Release();
	_planarPixelShader->Release();

	planarVertexShaderByteCode->Release();
	planarPixelShaderByteCode->Release();

	_CBReflect->Release();
	_CBVertex->Release();

	delete _simpleClipShaderReflection;
}

void PlanarReflectionShader::ShowDebugQuads()
{
	Systems::renderer->debugQuadHandler->AddDebugQuad(_simpleClipShaderReflection->GetRenderSRV());
}

void PlanarReflectionShader::Render(std::vector<Mesh*>& reflectionMeshes)
{
	if (reflectionMeshes.size() == 0)
		return;

	// get systems
	DXManager&     DXM = *Systems::dxManager;
	CameraManager& CM  = *Systems::cameraManager;
	LightManager&  LM  = *Systems::lightManager;
	Renderer& renderer = *Systems::renderer;

	// get device context and cameras
	ID3D11DeviceContext*& devCon  = DXM.devCon;
	CameraComponent*& camera      = CM.currentCameraGame;
	CameraComponent*& cameraLight = CM.currentCameraDepthMap;

	TransformComponent* camTrans = camera->GetComponent<TransformComponent>();

	// save camera properties before we change the 
	// position and rotation to render the reflectionmap
	XMFLOAT3 cameraPos = camTrans->position;
	XMFLOAT3 cameraRot = camTrans->rotation;

	// create constant buffer structures
	CBVertex  cbVertex;
	CBReflect cbReflect;

	XMStoreFloat3(&cbVertex.camPos, XMLoadFloat3(&cameraPos));

	// loop over all meshes that will project reflections onto itself
	size_t numMeshes = reflectionMeshes.size();
	for (int i = 0; i < numMeshes; i++)
	{
		// get the mesh that will project the reflections
		Mesh* mesh = reflectionMeshes[i];

		// get the clip plane of the reflection mesh so we dont render anything below this plane
		XMFLOAT4 clipPlane(0, 1, 0, -mesh->GetPosition().y);

		// change the camera y position to render reflection
		XMFLOAT3 reflectPos = cameraPos;
		reflectPos.y -= (cameraPos.y - mesh->GetPosition().y) * 2.0f;

		// set reflection position/rotation and build matrices
		camTrans->rotation = XMFLOAT3(-cameraRot.x, cameraRot.y, cameraRot.z);
		camTrans->position = reflectPos;
		camTrans->BuildWorldMatrix();
		camera->CalculateViewMatrix();

		// save the reflection viewprojmatrix so we can upload it to constant buffer
		XMFLOAT4X4 reflectMat = camera->viewProjMatrix;

		// render the reflectionmap
		_simpleClipShaderReflection->RenderScene(renderer.GetMeshes(SHADER_TYPE::S_CAST_REFLECTION_OPAQUE), renderer.GetMeshes(SHADER_TYPE::S_CAST_REFLECTION_ALPHA), renderer.GetInstancedModels(INSTANCED_SHADER_TYPE::S_INSTANCED_CAST_REFLECTION), clipPlane, true);

		// change back to original camera position/rotation
		camTrans->rotation = cameraRot;
		camTrans->position = cameraPos;
		camTrans->BuildWorldMatrix();
		camera->CalculateViewMatrix();

		// set back to defualt render target and render the reflection mesh
		Systems::renderer->SetMainRenderTarget();

		// set shaders
		devCon->VSSetShader(_planarVertexShader, NULL, 0);
		devCon->PSSetShader(_planarPixelShader, NULL, 0);

		// set the vertex constant buffer, the pixel ones is already set
		devCon->VSSetConstantBuffers(0, 1, &_CBVertex);
		devCon->PSSetConstantBuffers(0, 1, &LM.cbAmbDir);
		devCon->PSSetConstantBuffers(1, 1, &LM.cbPoint);
		devCon->PSSetConstantBuffers(2, 1, &_CBReflect);

		// set to alpha blending
		DXM.blendStates->SetBlendState(BLEND_STATE::BLEND_ALPHA);

		// set vertex constant values
		const XMFLOAT4X4& worldMat = mesh->GetWorldMatrix();

		XMStoreFloat4x4(&cbVertex.world,                XMLoadFloat4x4(&mesh->GetWorldMatrixTrans()));
		XMStoreFloat4x4(&cbVertex.worldViewProj,        XMLoadFloat4x4(&MATH_HELPERS::MatrixMutiplyTrans(&worldMat, &camera->viewProjMatrix)));
		XMStoreFloat4x4(&cbVertex.worldViewProjLight,   XMLoadFloat4x4(&MATH_HELPERS::MatrixMutiplyTrans(&worldMat, &cameraLight->viewProjMatrix)));
		XMStoreFloat4x4(&cbVertex.worldViewProjReflect, XMLoadFloat4x4(&MATH_HELPERS::MatrixMutiplyTrans(&worldMat, &reflectMat)));
		XMStoreFloat2(&cbVertex.uvOffset,               XMLoadFloat2(&mesh->uvOffset));

		// set the fraction of the reflection blending with the texture color
		cbReflect.reflectiveFraction = mesh->planarReflectionFraction;

		// update vertex constant buffer
		SHADER_HELPERS::UpdateConstantBuffer((void*)&cbVertex,  sizeof(CBVertex),  _CBVertex);
		SHADER_HELPERS::UpdateConstantBuffer((void*)&cbReflect, sizeof(CBReflect), _CBReflect);

		// fill texture array with all textures including the shadow map and reflection map
		ID3D11ShaderResourceView* t[6] = { mesh->baseTextures[0], mesh->baseTextures[1], mesh->baseTextures[2], mesh->baseTextures[3], cameraLight->renderTexture, _simpleClipShaderReflection->GetRenderSRV() };

		// set SRV's
		devCon->PSSetShaderResources(0, 6, t);

		mesh->UploadBuffers();

		devCon->DrawIndexed(mesh->numIndices, 0, 0);
	}

	// unbind so we can use resources as input in next stages
	ID3D11ShaderResourceView* nullSRV[6] = { NULL, NULL, NULL, NULL, NULL, NULL };
	devCon->PSSetShaderResources(0, 6, nullSRV);
}
