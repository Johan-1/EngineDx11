#include "QuadShader.h"
#include "DXManager.h"
#include "QuadComponent.h"
#include "CameraComponent.h"
#include "CameraManager.h"
#include "ShaderHelpers.h"
#include "DXBlendStates.h"
#include "DXDepthStencilStates.h"
#include "Systems.h"
#include "MathHelpers.h"

QuadShader::QuadShader()
{
	// create quad shaders
	SHADER_HELPERS::CreateVertexShader(L"Data/shaders/vertexQuad.shader", _vertexShader, vertexShaderByteCode);
	SHADER_HELPERS::CreatePixelShader(L"Data/shaders/pixelQuad.shader",   _pixelShader,  pixelShaderByteCode);

	// create constant buffers
	SHADER_HELPERS::CreateConstantBuffer(_constantBufferVertex);
	SHADER_HELPERS::CreateConstantBuffer(_constantBufferPixel);
}

QuadShader::~QuadShader()
{
	vertexShaderByteCode->Release();
	pixelShaderByteCode->Release();

	_vertexShader->Release();
	_pixelShader->Release();

	_constantBufferPixel->Release();
	_constantBufferVertex->Release();
}

void QuadShader::RenderQuadUI(const std::vector<QuadComponent*>& quads)
{
	if (quads.size() == 0)
		return;

	// get DXManager
	DXManager& DXM = *Systems::dxManager;

	// get device context
	ID3D11DeviceContext*& devCon = DXM.devCon;

	// set shaders			
	devCon->VSSetShader(_vertexShader, NULL, 0);
	devCon->PSSetShader(_pixelShader,  NULL, 0);

	// set constant buffers
	devCon->VSSetConstantBuffers(0, 1, &_constantBufferVertex);
	devCon->PSSetConstantBuffers(0, 1, &_constantBufferPixel);

	// render with alpha blending
	DXM.blendStates->SetBlendState(BLEND_STATE::BLEND_ALPHA);

	// render with depth off
	DXM.depthStencilStates->SetDepthStencilState(DEPTH_STENCIL_STATE::DISABLED);
	
	// constant buffer structures
	ConstantQuadUIVertex vertexData;
	ConstantQuadUIPixel  pixelData;

	// fill constant buffer vertex with the camera matrices
	XMFLOAT4X4 cameraMatrix = Systems::cameraManager->currentCameraUI->viewProjMatrix;

	for (int i = 0; i < quads.size(); i++)
	{
		// set the color of this quad in the constant pixel buffer
		pixelData.color       = quads[i]->color;
		pixelData.ignoreAlpha = quads[i]->ignoreAlpha;

		XMStoreFloat4x4(&vertexData.worldViewProj, XMLoadFloat4x4(&MATH_HELPERS::MatrixMutiplyTrans(&quads[i]->GetWorldMatrix(), &cameraMatrix)));

		// update the buffers for each quad
		SHADER_HELPERS::UpdateConstantBuffer((void*)&vertexData, sizeof(ConstantQuadUIVertex), _constantBufferVertex);
		SHADER_HELPERS::UpdateConstantBuffer((void*)&pixelData, sizeof(ConstantQuadUIPixel), _constantBufferPixel);

		// set the texture
		devCon->PSSetShaderResources(0, 1, &quads[i]->texture);

		// set the vertex and inex buffers
		quads[i]->UploadBuffers();

		// draw
		devCon->DrawIndexed(6, 0, 0);
	}

	// enable depth after the UI Rendering is done
	DXM.depthStencilStates->SetDepthStencilState(DEPTH_STENCIL_STATE::ENABLED);
}
