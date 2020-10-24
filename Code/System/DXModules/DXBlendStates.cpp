#include "DXBlendStates.h"
#include <iostream>
#include "DXErrorHandler.h"

DXBlendStates::DXBlendStates(ID3D11Device* device, ID3D11DeviceContext* devCon) :
	_device(device),
	_devCon(devCon)
{
	CreateBlendStates();
}

DXBlendStates::~DXBlendStates()
{
	_blendStateAlpha->Release();
	_blendStateOpaque->Release();
	_blendStateAdditive->Release();
	_blendStateSubtractive->Release();
}

// create all blend states
void DXBlendStates::CreateBlendStates()
{
	HRESULT result;
	D3D11_BLEND_DESC blendStateDesc;

	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));

	// alpha blending
	blendStateDesc.RenderTarget[0].BlendEnable           = true;
	blendStateDesc.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	result = _device->CreateBlendState(&blendStateDesc, &_blendStateAlpha);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create blend state");
		
	// opaque blending
	blendStateDesc.RenderTarget[0].BlendEnable = false;

	result = _device->CreateBlendState(&blendStateDesc, &_blendStateOpaque);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create blend state");

	//additive blending
	blendStateDesc.RenderTarget[0].BlendEnable    = true;
	blendStateDesc.RenderTarget[0].SrcBlend       = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlend      = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;

	result = _device->CreateBlendState(&blendStateDesc, &_blendStateAdditive);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create blend state");

	// subtractive blending
	blendStateDesc.RenderTarget[0].BlendEnable    = true;
	blendStateDesc.RenderTarget[0].SrcBlend       = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlend      = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_REV_SUBTRACT;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;

	result = _device->CreateBlendState(&blendStateDesc, &_blendStateSubtractive);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create blend state");
}

// set blend state
void DXBlendStates::SetBlendState(BLEND_STATE state)
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	switch (state)
	{
	case BLEND_STATE::BLEND_OPAQUE:
		_devCon->OMSetBlendState(_blendStateOpaque, blendFactor, 0xfffffff);
		break;
	case BLEND_STATE::BLEND_ALPHA:
		_devCon->OMSetBlendState(_blendStateAlpha, blendFactor, 0xfffffff);
		break;
	case BLEND_STATE::BLEND_ADDITIVE:
		_devCon->OMSetBlendState(_blendStateAdditive, blendFactor, 0xfffffff);
		break;
	case BLEND_STATE::BLEND_SUBTRACTIVE:
		_devCon->OMSetBlendState(_blendStateSubtractive, blendFactor, 0xfffffff);
		break;
	}
}
