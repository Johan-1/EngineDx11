#include "WireframeShader.h"
#include "DXManager.h"
#include "CameraComponent.h"
#include "CameraManager.h"
#include "Mesh.h"
#include "ShaderHelpers.h"
#include "DXRasterizerStates.h"
#include "Systems.h"
#include "MathHelpers.h"

WireframeShader::WireframeShader()
{
	// create and compile shaders
	SHADER_HELPERS::CreateVertexShader(L"Data/shaders/vertexWireframe.shader", _vertexShader, vertexShaderByteCode);
	SHADER_HELPERS::CreatePixelShader(L"Data/shaders/pixelWireframe.shader",   _pixelShader,  pixelShaderByteCode);
	 
	// create constant buffer for vertex shader
	SHADER_HELPERS::CreateConstantBuffer(_constantBufferVertex);
}

WireframeShader::~WireframeShader()
{
	vertexShaderByteCode->Release();
	pixelShaderByteCode->Release();

	_vertexShader->Release();
	_pixelShader->Release();

	_constantBufferVertex->Release();
}

void WireframeShader::RenderWireFrame(std::vector<Mesh*>& meshes)
{
	if (meshes.size() == 0)
		return;

	// get dx and camera manager
	DXManager& DXM    = *Systems::dxManager;
	CameraManager& CM = *Systems::cameraManager;

	// get devicecontext
	ID3D11DeviceContext*& devCon = DXM.devCon;

	// constantbuffer structure for vertex data
	ConstantVertex vertexData;

	// get the camera that will render the depthmap
	CameraComponent* camera = CM.currentCameraGame;

	// set our shaders
	devCon->VSSetShader(_vertexShader, NULL, 0);
	devCon->PSSetShader(_pixelShader, NULL, 0);

	// set the vertex constant buffer
	devCon->VSSetConstantBuffers(0, 1, &_constantBufferVertex);

	DXM.rasterizerStates->SetRasterizerState(RASTERIZER_STATE::WIREFRAME);

	size_t size = meshes.size();
	for (int i = 0; i < size; i++)
	{
		// set world matrix
		XMStoreFloat4x4(&vertexData.worldViewProj, XMLoadFloat4x4(&MATH_HELPERS::MatrixMutiplyTrans(&meshes[i]->GetWorldMatrix(), &camera->viewProjMatrix)));

		// update the constant buffer with the vertexdata of this mesh
		SHADER_HELPERS::UpdateConstantBuffer((void*)&vertexData, sizeof(ConstantVertex), _constantBufferVertex);

		// upload vertices and indices
		meshes[i]->UploadBuffers();

		// draw
		devCon->DrawIndexed(meshes[i]->numIndices, 0, 0);
	}

	DXM.rasterizerStates->SetRasterizerState(RASTERIZER_STATE::BACKCULL);
}
