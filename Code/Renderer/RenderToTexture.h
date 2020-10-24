#pragma once
#include <d3d11.h>

class RenderToTexture
{
public:
	RenderToTexture(unsigned int width, unsigned int height, bool depthOnly, bool HDR, bool addRenderDebugQuad = false, bool addDepthDebugQuad = false);
	~RenderToTexture();

	// clear this render target
	void ClearRenderTarget(float r,float g, float b, float a, bool depthOnly);

	// set this rendertarget
	void SetRendertarget(bool depthOnly, bool depthReadOnly);

	ID3D11RenderTargetView*   renderTargetView;
	ID3D11DepthStencilView*   depthStencilView;
	ID3D11DepthStencilView*   depthStencilViewReadOnly;
	ID3D11ShaderResourceView* renderTargetSRV;
	ID3D11ShaderResourceView* depthStencilSRV;
	D3D11_VIEWPORT            viewport;
};

