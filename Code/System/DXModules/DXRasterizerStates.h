#pragma once
#include <D3D11.h>

// defines available rasterizer states
enum RASTERIZER_STATE
{
	BACKCULL,
	FRONTCULL,
	NOCULL,
	WIREFRAME
};

class DXRasterizerStates
{
public:
	DXRasterizerStates(ID3D11Device* device, ID3D11DeviceContext* devCon);
	~DXRasterizerStates();

	// set a rasterizer state
	void SetRasterizerState(RASTERIZER_STATE state);

private:

	// creates the rasterizer states
	void CreateRasterizerStates();

	// pointers to directX interfaces
	ID3D11Device*        _device;
	ID3D11DeviceContext* _devCon;

	// rasterizer states
	ID3D11RasterizerState* _rasterizerBackCull;
	ID3D11RasterizerState* _rasterizerFrontCull;
	ID3D11RasterizerState* _rasterizerNoCull;
	ID3D11RasterizerState* _rasterizerWireframe;
};

