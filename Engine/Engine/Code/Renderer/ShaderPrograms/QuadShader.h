#pragma once
#include <D3D11.h>
#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

class QuadComponent;

class QuadShader
{
public:
	QuadShader();
	~QuadShader();

	void RenderQuadUI(const std::vector<QuadComponent*>& quads);

	// the shader bytecode
	ID3D10Blob* vertexShaderByteCode;
	ID3D10Blob* pixelShaderByteCode;

private:

	// constant buffers
	ID3D11Buffer* _constantBufferVertex;
	ID3D11Buffer* _constantBufferPixel;

	// compiled shaders
	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader*  _pixelShader;

	// vertex canstant structure
	struct ConstantQuadUIVertex
	{
		XMFLOAT4X4 worldViewProj;
	};

	// pixel constant structure
	struct ConstantQuadUIPixel
	{
		XMFLOAT4 color;
		int ignoreAlpha;
		XMFLOAT3 pad;
	};
};

