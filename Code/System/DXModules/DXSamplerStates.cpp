#include "DXSamplerStates.h"
#include <iostream>
#include "DXErrorHandler.h"

DXSamplerStates::DXSamplerStates(ID3D11Device* device, ID3D11DeviceContext* devCon):
	_device(device),
	_devCon(devCon)
{
	CreateSamplerStates();
}

DXSamplerStates::~DXSamplerStates()
{
	_sampleStateWrapTri->Release();
	_sampleStateClampTri->Release();
	_sampleStateWrapBil->Release();
	_sampleStateClampBil->Release();
	_sampleStateWrapAni->Release();
	_sampleStateClampAni->Release();
}

void DXSamplerStates::CreateSamplerStates()
{
	HRESULT result;
	D3D11_SAMPLER_DESC samplerDesc;

	// create sampler state wrap
	samplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias     = 0.0f;
	samplerDesc.MaxAnisotropy  = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD         = 0;
	samplerDesc.MaxLOD         = D3D11_FLOAT32_MAX;

	// trilinear
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	result = _device->CreateSamplerState(&samplerDesc, &_sampleStateWrapTri);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create sampler state wrap");

	// trilinear
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	result = _device->CreateSamplerState(&samplerDesc, &_sampleStateWrapBil);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create sampler state wrap");

	// anisotropic
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	result = _device->CreateSamplerState(&samplerDesc, &_sampleStateWrapAni);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create sampler state wrap");

	// create sampler state clamp
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	// trilinear
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	result = _device->CreateSamplerState(&samplerDesc, &_sampleStateClampTri);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create sampler state wrap");

	// trilinear
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	result = _device->CreateSamplerState(&samplerDesc, &_sampleStateClampBil);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create sampler state wrap");

	// anisotropic
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	result = _device->CreateSamplerState(&samplerDesc, &_sampleStateClampAni);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create sampler state wrap");

	// set these samplers to be used in the pixelshaders
	ID3D11SamplerState* samplers[6]{ _sampleStateWrapTri, _sampleStateClampTri, _sampleStateWrapBil, _sampleStateClampBil, _sampleStateWrapAni, _sampleStateClampAni };
	_devCon->PSSetSamplers(0, 6, samplers);
}
