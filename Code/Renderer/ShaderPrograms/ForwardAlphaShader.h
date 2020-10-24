#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

class Mesh;

class ForwardAlphaShader
{
public:
	ForwardAlphaShader();
	~ForwardAlphaShader();

	void RenderForward(std::vector<Mesh*>& meshes);

	// the shader bytecode
	ID3D10Blob* vertexShaderByteCode;
	ID3D10Blob* pixelShaderByteCode;

private:

	// compiled shaders
	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader*  _pixelShader;

	// constant buffers
	ID3D11Buffer* _CBVertex;
	ID3D11Buffer* _CBPixel;

	struct CBVertex
	{
		XMFLOAT4X4 world;
		XMFLOAT4X4 worldViewProj;
		XMFLOAT4X4 worldViewProjLight;

		XMFLOAT3 camPos;
		float    pad1;
		XMFLOAT2 uvOffset;
		XMFLOAT2 pad2;
	};

	struct CBPixel
	{
		int      hasHeightmap;
		float    heightScale;
		XMFLOAT2 pad;
	};
};

