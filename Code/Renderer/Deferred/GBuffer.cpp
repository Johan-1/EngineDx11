#include "GBuffer.h"
#include "Systemdefs.h"
#include "DXManager.h"
#include <iostream>
#include "Systems.h"
#include "DXErrorHandler.h"

GBuffer::GBuffer()
{
	CreateRenderTargets();
}

GBuffer::~GBuffer()
{
}

void GBuffer::SetRenderTargets(ID3D11DepthStencilView* depthStencil)
{
	DXManager& dXM               = *Systems::dxManager;
	ID3D11DeviceContext*& devCon = dXM.devCon;
	
	// clear to black
	float clear[4] = { 0,0,0,1 };

	// clear all targets from last frame
	for(int i =0; i < 4; i++)
		devCon->ClearRenderTargetView(_renderTargetArray[i], clear);

	// set default viewport
	dXM.SetDefaultViewport();

	// set the GBuffer
	devCon->OMSetRenderTargets(4, _renderTargetArray, depthStencil);
}

void GBuffer::CreateRenderTargets()
{
	// get device
	DXManager& DXM        = *Systems::dxManager;
	ID3D11Device*& device = DXM.device;

	// allocate array of pointers
	_renderTargetArray = new ID3D11RenderTargetView*[4];
	SRVArray           = new ID3D11ShaderResourceView*[4];	

	// texture that we will use to create targets
	ID3D11Texture2D* tex2D;

	HRESULT result;

	// texture description for render targets
	// some of this data will change depending on the render target we are creating
	D3D11_TEXTURE2D_DESC RenderTargetTexDesc;
	ZeroMemory(&RenderTargetTexDesc, sizeof(D3D11_TEXTURE2D_DESC));
	RenderTargetTexDesc.Width            = SystemSettings::SCREEN_WIDTH;
	RenderTargetTexDesc.Height           = SystemSettings::SCREEN_HEIGHT;
	RenderTargetTexDesc.MipLevels        = 1;
	RenderTargetTexDesc.ArraySize        = 1;
	RenderTargetTexDesc.Format           = DXGI_FORMAT_R32G32B32A32_FLOAT;
	RenderTargetTexDesc.SampleDesc.Count = 1;
	RenderTargetTexDesc.Usage            = D3D11_USAGE_DEFAULT;
	RenderTargetTexDesc.BindFlags        = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	RenderTargetTexDesc.CPUAccessFlags   = 0;
	RenderTargetTexDesc.MiscFlags        = 0;

	// description for the render target view
	// all render targets share this
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	ZeroMemory(&renderTargetViewDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	renderTargetViewDesc.Format             = RenderTargetTexDesc.Format;
	renderTargetViewDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// desxription for the SRV's 
	// all srv's share this
	D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
	ZeroMemory(&resourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	resourceViewDesc.Format                    = RenderTargetTexDesc.Format;
	resourceViewDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	resourceViewDesc.Texture2D.MostDetailedMip = 0;
	resourceViewDesc.Texture2D.MipLevels       = 1;	

	// create the texture and views for position buffer
	// position use 16 byte (optimise so we reconstruct position from depth, this whole buffer can then be removed)
	result = device->CreateTexture2D(&RenderTargetTexDesc, NULL, &tex2D);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create texture2D for GBuffer position");
	
	result = device->CreateRenderTargetView(tex2D, &renderTargetViewDesc, &_renderTargetArray[0]);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create render target view for GBuffer position");

	result = device->CreateShaderResourceView(tex2D, &resourceViewDesc, &SRVArray[0]);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create SRV for GBuffer position");

	tex2D->Release();
	
	// update descriptions for the normal views
	// 8 bytes for normal buffer
	RenderTargetTexDesc.Format  = DXGI_FORMAT_R16G16B16A16_FLOAT;
	renderTargetViewDesc.Format = RenderTargetTexDesc.Format;
	resourceViewDesc.Format     = RenderTargetTexDesc.Format;

	// create the texture and views for normal buffer
	result = device->CreateTexture2D(&RenderTargetTexDesc, NULL, &tex2D);
	if (FAILED(result))
		DX_ERROR::PrintError(result, "failed to create texture2D for GBuffer normal");
	
	result = device->CreateRenderTargetView(tex2D, &renderTargetViewDesc, &_renderTargetArray[1]);
	if (FAILED(result))
		DX_ERROR::PrintError(result, "failed to create render target view for GBuffer normal");

	result = device->CreateShaderResourceView(tex2D, &resourceViewDesc, &SRVArray[1]);
	if (FAILED(result))
		DX_ERROR::PrintError(result, "failed to create SRV for GBuffer normal");

	tex2D->Release();

	// 4 byte for both diffuse and specular buffers
	RenderTargetTexDesc.Format  = DXGI_FORMAT_R8G8B8A8_UNORM;
	renderTargetViewDesc.Format = RenderTargetTexDesc.Format;
	resourceViewDesc.Format     = RenderTargetTexDesc.Format;

	// create the texture and views for diffuse and specular buffers
	for (int i = 2; i< 4; i++)
	{
		result = device->CreateTexture2D(&RenderTargetTexDesc, NULL, &tex2D);
		if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create texture2D for diffuse/specualr GBuffer");

		result = device->CreateRenderTargetView(tex2D, &renderTargetViewDesc, &_renderTargetArray[i]);
		if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create render target view for diffuse/specualr GBuffer");

		result = device->CreateShaderResourceView(tex2D, &resourceViewDesc, &SRVArray[i]);
		if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create SRV for diffuse/specualr GBuffer");

		tex2D->Release();
	}	
}
