#pragma once
#include <d3d11.h>

// defines our input Layouts
enum INPUT_LAYOUT_TYPE
{
	LAYOUT_3D,
	LAYOUT_2D,
	LAYOUT_PARTICLE,
	LAYOUT_GUI,
	LAYOUT_3D_INSTANCED,
};

class DXInputLayouts
{
public:
	DXInputLayouts();
	~DXInputLayouts();

	// creates input layouts
	void CreateInputLayout3D(ID3D10Blob*& vertexShaderByteCode);
	void CreateInputLayout3DInstanced(ID3D10Blob*& vertexShaderByteCode);
	void CreateInputLayout2D(ID3D10Blob*& vertexShaderByteCode);
	void CreateInputLayoutParticle(ID3D10Blob*& vertexShaderByteCode);
	void CreateInputLayoutGUI(ID3D10Blob*& vertexShaderByteCode);

	// set layout based on enum type
	void SetInputLayout(INPUT_LAYOUT_TYPE type);

private:

	// the input layouts
	ID3D11InputLayout* _inputLayout3D;
	ID3D11InputLayout* _inputLayout3DInstanced;
	ID3D11InputLayout* _inputLayout2D;
	ID3D11InputLayout* _inputLayoutParticle;
	ID3D11InputLayout* _inputLayoutGUI;
};

