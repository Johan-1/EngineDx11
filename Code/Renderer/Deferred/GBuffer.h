#pragma once
#include <d3d11.h>

class GBuffer
{
public:
	GBuffer();
	~GBuffer();

	// sets the rendertargets of this buffer
	void SetRenderTargets(ID3D11DepthStencilView* depthStencil);

	ID3D11ShaderResourceView** SRVArray;

private:

	// creates all rander targets
	void CreateRenderTargets();

	// srv and render target arrays
	ID3D11RenderTargetView**   _renderTargetArray;
};

