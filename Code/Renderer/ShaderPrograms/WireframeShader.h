#pragma once
#include <d3d11.h>
#include <vector>
#include <DirectXMath.h>

class Mesh;

using namespace DirectX;

class WireframeShader
{
public:
	WireframeShader();
	~WireframeShader();

	// renders all meshes to the depth map
	void RenderWireFrame(std::vector<Mesh*>& meshes);

	// the shader bytecode
	ID3D10Blob* vertexShaderByteCode;
	ID3D10Blob* pixelShaderByteCode;

private:

	// compiled shaders
	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader*  _pixelShader;

	// constant buffers
	ID3D11Buffer* _constantBufferVertex;

	// constant data structure
	struct ConstantVertex
	{
		XMFLOAT4X4 worldViewProj;
	};
};

