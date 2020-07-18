#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

#define PLANAR_BLUR_SCALE 2.0f
#define USE_BLURRED_REFELECTIONS true

using namespace DirectX;

class Mesh;
class ParticleSystemComponent;
class RenderToTexture;
class ParticleShader;
class DXInputLayouts;
class SimpleClipSceneShader;

class PlanarReflectionShader
{
public:
	PlanarReflectionShader();
	~PlanarReflectionShader();

	void Render(std::vector<Mesh*>& reflectionMeshes);
	void ShowDebugQuads();

	// the shader bytecode
	ID3D10Blob* planarVertexShaderByteCode;
	ID3D10Blob* planarPixelShaderByteCode;

private:
	
	RenderToTexture* _blurH;
	RenderToTexture* _blurV;

	// compiled shaders
	ID3D11VertexShader*    _planarVertexShader;
	ID3D11PixelShader*     _planarPixelShader;
	SimpleClipSceneShader* _simpleClipShaderReflection;

	// constant buffers
	ID3D11Buffer* _CBVertex;
	ID3D11Buffer* _CBReflect;

	struct CBVertex
	{
		XMFLOAT4X4 world;
		XMFLOAT4X4 worldViewProj;
		XMFLOAT4X4 worldViewProjLight;
		XMFLOAT4X4 worldViewProjReflect;

		XMFLOAT3 camPos;
		float    pad1;
		XMFLOAT2 uvOffset;
		XMFLOAT2 pad2;
	};

	struct CBReflect
	{
		float    reflectiveFraction;
		XMFLOAT3 pad;
	};
};

