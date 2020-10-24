#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

class ParticleSystemComponent;

class ParticleShader
{
public:
	ParticleShader();
	~ParticleShader();

	// render all particle systems
	void RenderParticles(std::vector<ParticleSystemComponent*>& systems);

	// the shader bytecode
	ID3D10Blob* vertexShaderByteCode;
	ID3D10Blob* pixelShaderByteCode;

private:

	// the shaders
	ID3D11VertexShader* _vertexShader;
	ID3D11PixelShader*  _pixelShader;

	// constant vertex buffer
	ID3D11Buffer* _constantBufferVertex;

	// constant vertex structure
	struct CBVertex
	{
		XMFLOAT4X4 viewProj;
	};
};

