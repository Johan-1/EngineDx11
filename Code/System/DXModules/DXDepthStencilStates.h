#pragma once
#include <D3D11.h>

// will set all geamoetry pixels in the depth stencil
// the skybox and deferred lightning pass will later
// mask out all of these pixels or mask out all other pixels
// during thier render pass
#define STENCIL_GEOMETRY_PIXELS 2

// available depth stencil states
enum DEPTH_STENCIL_STATE
{
	ENABLED,
	DISABLED,
	READ_ONLY,
	MASKED_SKYBOX,
	MASKED_LIGHTNING
};

class DXDepthStencilStates
{
public:
	DXDepthStencilStates(ID3D11Device* device, ID3D11DeviceContext* devCon);
	~DXDepthStencilStates();

	// sets a depth stencil
	void SetDepthStencilState(DEPTH_STENCIL_STATE state);

private:

	// creates the rasterizer states
	void CreateDepthStencilStates();

	// pointers to directX interfaces
	ID3D11Device*        _device;
	ID3D11DeviceContext* _devCon;

	// depth stencil states
	ID3D11DepthStencilState* _depthStencilEnabled;
	ID3D11DepthStencilState* _depthStencilDisabled;
	ID3D11DepthStencilState* _depthStencilReadOnly;
	ID3D11DepthStencilState* _depthStencilMaskedSkybox;
	ID3D11DepthStencilState* _depthStencilMaskedLightning;
};

