#include "ForwardAlphaShader.h"
#include "ShaderHelpers.h"
#include "DXManager.h"
#include "DXBlendStates.h"
#include "DXDepthStencilStates.h"
#include "CameraManager.h"
#include "LightManager.h"
#include "Mesh.h"
#include "Systems.h"
#include "MathHelpers.h"

ForwardAlphaShader::ForwardAlphaShader()
{
	//create shaders
	SHADER_HELPERS::CreateVertexShader(L"Data/shaders/vertexForward.shader",  _vertexShader, vertexShaderByteCode);
	SHADER_HELPERS::CreatePixelShader(L"Data/shaders/pixelForward.shader",    _pixelShader,  pixelShaderByteCode);

	// create constant buffers
	SHADER_HELPERS::CreateConstantBuffer(_CBVertex);
	SHADER_HELPERS::CreateConstantBuffer(_CBPixel);
}

ForwardAlphaShader::~ForwardAlphaShader()
{
	_vertexShader->Release();
	_pixelShader->Release();

	vertexShaderByteCode->Release();
	pixelShaderByteCode->Release();

	_CBVertex->Release();
	_CBPixel->Release();
}

void ForwardAlphaShader::RenderForward(std::vector<Mesh*>& meshes)
{
	if (meshes.size() == 0)
		return;

	// get systems
	DXManager& DXM    = *Systems::dxManager;
	CameraManager& CM = *Systems::cameraManager;
	LightManager& LM  = *Systems::lightManager;

	// get context and cameras
	ID3D11DeviceContext*& devCon  = DXM.devCon;
	CameraComponent*& camera      = CM.currentCameraGame;
	CameraComponent*& cameraLight = CM.currentCameraDepthMap;

	// get camera position
	const XMFLOAT3& cameraPos = camera->GetComponent<TransformComponent>()->position;

	// set shaders
	devCon->VSSetShader(_vertexShader, NULL, 0);
	devCon->PSSetShader(_pixelShader, NULL, 0);

	// set constant buffers
	devCon->VSSetConstantBuffers(0, 1, &_CBVertex);
	devCon->PSSetConstantBuffers(0, 1, &LM.cbAmbDir);
	devCon->PSSetConstantBuffers(1, 1, &LM.cbPoint);
	devCon->PSSetConstantBuffers(2, 1, &_CBPixel);

	// set to alpha blending
	DXM.blendStates->SetBlendState(BLEND_STATE::BLEND_ALPHA);

	// create constant buffer structures
	CBVertex constantVertex;
	CBPixel  constantPixel;

	// set camera matrices
	XMStoreFloat3(&constantVertex.camPos, XMLoadFloat3(&cameraPos));

	// sort alpha meshes to render back to front
	SHADER_HELPERS::MeshSort(meshes, cameraPos, true);

	size_t numMeshes = meshes.size();
	for (int i = 0; i < numMeshes; i++)
	{
		// get mesh
		Mesh* mesh = meshes[i];

		// get mesh specific constant data
		const XMFLOAT4X4& worldMat = mesh->GetWorldMatrix();

		XMStoreFloat4x4(&constantVertex.world,              XMLoadFloat4x4(&mesh->GetWorldMatrixTrans()));
		XMStoreFloat4x4(&constantVertex.worldViewProj,      XMLoadFloat4x4(&MATH_HELPERS::MatrixMutiplyTrans(&worldMat, &camera->viewProjMatrix)));
		XMStoreFloat4x4(&constantVertex.worldViewProjLight, XMLoadFloat4x4(&MATH_HELPERS::MatrixMutiplyTrans(&worldMat, &cameraLight->viewProjMatrix)));
		XMStoreFloat2(&constantVertex.uvOffset,             XMLoadFloat2(&mesh->uvOffset));

		constantPixel.hasHeightmap = mesh->hasHeightmap;
		constantPixel.heightScale  = mesh->heightMapScale;

		// update vertex constant buffer
		SHADER_HELPERS::UpdateConstantBuffer((void*)&constantVertex, sizeof(CBVertex), _CBVertex);
		SHADER_HELPERS::UpdateConstantBuffer((void*)&constantPixel,  sizeof(CBPixel),  _CBPixel);

		// fill texture array with all textures including the shadow map
		ID3D11ShaderResourceView* t[5] = { mesh->baseTextures[0], mesh->baseTextures[1], mesh->baseTextures[2], mesh->baseTextures[3], cameraLight->renderTexture };

		// set SRV's
		devCon->PSSetShaderResources(0, 5, t);

		mesh->UploadBuffers();

		devCon->DrawIndexed(mesh->numIndices, 0, 0);
	}

	// unbind so we can use resources as input in next stages
	ID3D11ShaderResourceView* nullSRV[5] = { NULL, NULL, NULL, NULL, NULL };
	devCon->PSSetShaderResources(0, 5, nullSRV);
}
