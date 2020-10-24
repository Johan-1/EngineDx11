#include "DXDepthStencilStates.h"
#include <iostream>
#include "DXErrorHandler.h"

DXDepthStencilStates::DXDepthStencilStates(ID3D11Device* device, ID3D11DeviceContext* devCon):
	_device(device),
	_devCon(devCon)
{
	CreateDepthStencilStates();
}

DXDepthStencilStates::~DXDepthStencilStates()
{
	_depthStencilEnabled->Release();
	_depthStencilDisabled->Release();
	_depthStencilReadOnly->Release();
	_depthStencilMaskedSkybox->Release();
	_depthStencilMaskedLightning->Release();
}

void DXDepthStencilStates::CreateDepthStencilStates()
{
	HRESULT result;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;

	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable      = true;
	depthStencilDesc.DepthWriteMask   = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc        = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilDesc.StencilEnable    = true;
	depthStencilDesc.StencilReadMask  = 0xFFFFFFFF;
	depthStencilDesc.StencilWriteMask = 0xFFFFFFFF;

	// Stencil operations
	// replace the stencil value with the reference value on the rendered pixel
	// the skybox will later mask the pixels that has the reference value set in the stencil
	depthStencilDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_REPLACE;
	depthStencilDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;

	depthStencilDesc.BackFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp      = D3D11_STENCIL_OP_REPLACE;
	depthStencilDesc.BackFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;

	// depth enabled
	result = _device->CreateDepthStencilState(&depthStencilDesc, &_depthStencilEnabled);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create depth stencil state");

	// will only read the depthbuffer, the depthstencil is ignored here aswell
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.StencilEnable  = true;
	depthStencilDesc.DepthEnable    = true;	

	result = _device->CreateDepthStencilState(&depthStencilDesc, &_depthStencilReadOnly);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create depth stencil state");

	// will not read or write the deptbuffer and stencilbuffer
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthEnable    = false;
	depthStencilDesc.StencilEnable  = false;

	result = _device->CreateDepthStencilState(&depthStencilDesc, &_depthStencilDisabled);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create depth stencil state");

	// masked depthstencil with reference set to the same as the regular depthEnabled depthstencil that is used while rendering geometry
	// comparison set to not equal meaning that the pixels that got flagged during the geometry pass will fail and skybox wont render over them
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthEnable    = false;
	depthStencilDesc.StencilEnable  = true;

	depthStencilDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_NOT_EQUAL;

	depthStencilDesc.BackFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc        = D3D11_COMPARISON_NOT_EQUAL;

	result = _device->CreateDepthStencilState(&depthStencilDesc, &_depthStencilMaskedSkybox);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create depth stencil state");

	// wont render non geometry pixels for the fullscreen light pass
	// only renders the pixels that got flagged during the geometrypass (opposite of skybox)
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	depthStencilDesc.BackFace.StencilFunc  = D3D11_COMPARISON_EQUAL;

	result = _device->CreateDepthStencilState(&depthStencilDesc, &_depthStencilMaskedLightning);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create depth stencil state");

	_devCon->OMSetDepthStencilState(_depthStencilEnabled, 1);
}

void DXDepthStencilStates::SetDepthStencilState(DEPTH_STENCIL_STATE state)
{
	switch (state)
	{
	case ENABLED:
		_devCon->OMSetDepthStencilState(_depthStencilEnabled, STENCIL_GEOMETRY_PIXELS);
		break;
	case DISABLED:
		_devCon->OMSetDepthStencilState(_depthStencilDisabled, 1);
		break;
	case READ_ONLY:
		_devCon->OMSetDepthStencilState(_depthStencilReadOnly, 1);
		break;
	case MASKED_SKYBOX:
		_devCon->OMSetDepthStencilState(_depthStencilMaskedSkybox, STENCIL_GEOMETRY_PIXELS);
		break;
	case MASKED_LIGHTNING:
		_devCon->OMSetDepthStencilState(_depthStencilMaskedLightning, STENCIL_GEOMETRY_PIXELS);
	}
}
