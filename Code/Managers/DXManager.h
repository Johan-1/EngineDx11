#pragma once
#include <D3D11.h>

class DXBlendStates;
class DXRasterizerStates;
class DXDepthStencilStates;
class DXSamplerStates;
class HardwareProperties;

class DXManager
{
public:
	DXManager();
	~DXManager();

	void Initialize(HWND hwnd,int screenWidth, int screenHeight, bool vSync, bool fullscreen);
	void ClearRenderTarget(float r, float g, float b, float a);
	void PresentScene();	

	void SetDefaultViewport();
	void SetBackBuffer();
	void SetNullRenderTarget();

	void SetFullscreen(bool fullscreen, bool toggle = false);

	// DX modules
	DXBlendStates*        blendStates;
	DXRasterizerStates*   rasterizerStates;
	DXDepthStencilStates* depthStencilStates;
	DXSamplerStates*      samplerStates;
	HardwareProperties*   hardwareProperties;

	ID3D11Device*        device;
	ID3D11DeviceContext* devCon;

private:

	// create swapchain and devices
	void CreateSwapchainAndRenderTarget(HWND hwnd, bool fullscreen, int screenWidth, int screenHeight);
	void CreateDepthStencilViews(int screenWidth, int screenHeight);
	void SetDebugLayerIgnores();
	
	// create the viewport
	void CreateViewport(int screenWidth, int screenHeight);
			
	// misc settings
	bool _vsyncEnabled;
	bool _fullscreen;

	// backbuffer stuff
	ID3D11RenderTargetView*   _renderTargetView;
	ID3D11DepthStencilView*   _depthStencilView;
	IDXGISwapChain*           _swapChain;

	// default viewport
	D3D11_VIEWPORT _viewport;
};

