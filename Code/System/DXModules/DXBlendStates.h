#pragma once
#include <D3D11.h>

// describes all available blend states
enum BLEND_STATE
{
	BLEND_OPAQUE,
	BLEND_ALPHA,
	BLEND_ADDITIVE,
	BLEND_SUBTRACTIVE
};

class DXBlendStates
{
public:
	DXBlendStates(ID3D11Device* device, ID3D11DeviceContext* devCon);
	~DXBlendStates();

	// sets a blend state
	void SetBlendState(BLEND_STATE state);

private:

	// create all blend states
	void CreateBlendStates();

	// all blend states
	ID3D11BlendState* _blendStateAlpha;
	ID3D11BlendState* _blendStateOpaque;
	ID3D11BlendState* _blendStateAdditive;
	ID3D11BlendState* _blendStateSubtractive;

	// pointers to the directx interfaces
	ID3D11Device*        _device;
	ID3D11DeviceContext* _devCon;
};

