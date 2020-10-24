#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

class ImGUIShader
{
public:
	ImGUIShader();
	~ImGUIShader();

	void RenderGUI();

	// get byte code from shaders
	ID3D10Blob*& GetVertexShaderByteCode() { return _vertexShaderByteCode; }
	ID3D10Blob*& GetPixelShaderByteCode()  { return _pixelShaderByteCode; }

private:

	// constant buffers
	ID3D11Buffer* _constantBufferVertex;

	// compiled shaders
	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader*  _pixelShader;

	// the shader bytecode
	ID3D10Blob* _vertexShaderByteCode;
	ID3D10Blob* _pixelShaderByteCode;

	// vertex constant structure
	struct ConstantVertex
	{
		float projection[4][4];
	};
};

