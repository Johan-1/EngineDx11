#include "DepthShader.h"
#include "DXManager.h"
#include "CameraComponent.h"
#include "CameraManager.h"
#include "Mesh.h"
#include "ShaderHelpers.h"
#include "Systems.h"
#include "DXBlendStates.h"
#include "MathHelpers.h"
#include "InstancedModel.h"

DepthShader::DepthShader()
{
	// create and compile shaders
	SHADER_HELPERS::CreateVertexShader(L"Data/shaders/vertexDepth.shader", _vertexShader, vertexShaderByteCode);
	SHADER_HELPERS::CreateVertexShader(L"Data/shaders/vertexDepthInstanced.shader", _vertexShaderInstanced, vertexShaderByteCodeInstanced);
	SHADER_HELPERS::CreatePixelShader(L"Data/shaders/pixelDepth.shader",   _pixelShader,  pixelShaderByteCode);

	// create constant buffer for vertex shader
	SHADER_HELPERS::CreateConstantBuffer(_constantBufferVertex);
}

DepthShader::~DepthShader()
{
	vertexShaderByteCode->Release();
	pixelShaderByteCode->Release();

	_vertexShader->Release();
	_pixelShader->Release();

	_constantBufferVertex->Release();
}

void DepthShader::RenderDepth(std::vector<Mesh*>& meshes)
{
	// get dx manager
	DXManager& DXM               = *Systems::dxManager;
	CameraManager& CM            = *Systems::cameraManager;
	CameraComponent*& camera     = CM.currentCameraDepthMap;
	ID3D11DeviceContext*& devCon = DXM.devCon;

	DXM.blendStates->SetBlendState(BLEND_STATE::BLEND_ALPHA);

	// set our shaders
	devCon->VSSetShader(_vertexShader, NULL, 0);
	devCon->PSSetShader(_pixelShader, NULL, 0);

	// set the vertex constant buffer
	devCon->VSSetConstantBuffers(0, 1, &_constantBufferVertex);

	// constantbuffer structure for vertex data
	ConstantVertex vertexData;

	size_t size = meshes.size();
	for (int i = 0; i < size; i++)
	{
		Mesh*& mesh = meshes[i];

		// set world matrix of mesh
		XMStoreFloat4x4(&vertexData.worldViewProj, XMLoadFloat4x4(&MATH_HELPERS::MatrixMutiplyTrans(&mesh->GetWorldMatrix(), &camera->viewProjMatrix)));

		// update the constant buffer with the vertexdata of this mesh
		SHADER_HELPERS::UpdateConstantBuffer((void*)&vertexData, sizeof(ConstantVertex), _constantBufferVertex);

		// set textures
		devCon->PSSetShaderResources(0, 1, mesh->baseTextures);

		// upload vertices and indices
		mesh->UploadBuffers();

		// draw
		devCon->DrawIndexed(mesh->numIndices, 0, 0);
	}

	// unbind so we can use resources as input in next stages
	ID3D11ShaderResourceView* nullSRV[1] = { NULL };
	devCon->PSSetShaderResources(0, 1, nullSRV);
}

void DepthShader::RenderDepthInstanced(std::vector<InstancedModel*>& models)
{
	// get data needed
	DXManager& DXM               = *Systems::dxManager;
	CameraManager& CM            = *Systems::cameraManager;
	ID3D11DeviceContext*& devCon = DXM.devCon;	
	CameraComponent*& camera     = CM.currentCameraDepthMap;
	
	DXM.blendStates->SetBlendState(BLEND_STATE::BLEND_ALPHA);

	// set our shaders
	devCon->VSSetShader(_vertexShaderInstanced, NULL, 0);
	devCon->PSSetShader(_pixelShader, NULL, 0);

	// set the vertex constant buffer
	devCon->VSSetConstantBuffers(0, 1, &_constantBufferVertex);

	// set world matrix of mesh
	ConstantVertex vertexData;
	XMStoreFloat4x4(&vertexData.worldViewProj, XMLoadFloat4x4(&camera->viewProjMatrixTrans));
	SHADER_HELPERS::UpdateConstantBuffer((void*)&vertexData, sizeof(ConstantVertex), _constantBufferVertex);

	size_t size = models.size();
	for (int i = 0; i < size; i++)
	{
		InstancedModel*& model = models[i];

		model->UploadInstances();

		std::vector<Mesh*>& meshes = model->meshes;
		size_t meshesSize = meshes.size();
		for (int y = 0; y < meshesSize; y++)
		{
			Mesh*& mesh = meshes[y];

			// Set the vertex buffer in slot 0
			devCon->IASetVertexBuffers(0, 1, &mesh->vertexBuffer, model->strides, model->offsets);

			// Set the index buffer 
			devCon->IASetIndexBuffer(mesh->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

			// set textures
			devCon->PSSetShaderResources(0, 1, mesh->baseTextures);

			// draw all instances of this mesh
			devCon->DrawIndexedInstanced(mesh->numIndices, model->numInstances, 0, 0, 0);
		}
	}

	ID3D11ShaderResourceView* nullSRV[1] = { NULL};
	devCon->PSSetShaderResources(0, 1, nullSRV);
}
