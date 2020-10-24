#include "DXRasterizerStates.h"
#include <iostream>
#include "DXErrorHandler.h"

DXRasterizerStates::DXRasterizerStates(ID3D11Device* device, ID3D11DeviceContext* devCon):
	_device(device),
	_devCon(devCon)
{
	CreateRasterizerStates();
}

DXRasterizerStates::~DXRasterizerStates()
{
	_rasterizerBackCull->Release();
	_rasterizerFrontCull->Release();
	_rasterizerNoCull->Release();
	_rasterizerWireframe->Release();
}

void DXRasterizerStates::CreateRasterizerStates()
{
	HRESULT result;
	D3D11_RASTERIZER_DESC rasterDesc;

	rasterDesc.AntialiasedLineEnable = true;
	rasterDesc.CullMode              = D3D11_CULL_BACK;
	rasterDesc.DepthBias             = 0;
	rasterDesc.DepthBiasClamp        = 0.0f;
	rasterDesc.DepthClipEnable       = true;
	rasterDesc.FillMode              = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable     = true;
	rasterDesc.ScissorEnable         = false;
	rasterDesc.SlopeScaledDepthBias  = 0.0f;

	// with backface culling
	result = _device->CreateRasterizerState(&rasterDesc, &_rasterizerBackCull);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create rasterizer state backcull");

	// with frontface culling
	rasterDesc.CullMode = D3D11_CULL_FRONT;
	result = _device->CreateRasterizerState(&rasterDesc, &_rasterizerFrontCull);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create rasterizer state frontcull");

	// with no cull
	rasterDesc.CullMode = D3D11_CULL_NONE;
	result = _device->CreateRasterizerState(&rasterDesc, &_rasterizerNoCull);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create rasterizer state no cull");

	// wireframe
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	result = _device->CreateRasterizerState(&rasterDesc, &_rasterizerWireframe);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create rasterizer state wireframe");

	// initialize to backface culling
	_devCon->RSSetState(_rasterizerBackCull);
}

// set a rasterizer state to use
void DXRasterizerStates::SetRasterizerState(RASTERIZER_STATE state)
{
	switch (state)
	{
	case RASTERIZER_STATE::BACKCULL:
		_devCon->RSSetState(_rasterizerBackCull);
		break;
	case RASTERIZER_STATE::FRONTCULL:
		_devCon->RSSetState(_rasterizerFrontCull);
		break;
	case RASTERIZER_STATE::NOCULL:
		_devCon->RSSetState(_rasterizerNoCull);
		break;
	case RASTERIZER_STATE::WIREFRAME:
		_devCon->RSSetState(_rasterizerWireframe);
		break;
	}
}


