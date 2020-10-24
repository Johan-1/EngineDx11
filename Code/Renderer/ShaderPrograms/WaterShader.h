#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

class Mesh;
class ParticleSystemComponent;
class RenderToTexture;
class ParticleShader;
class DXInputLayouts;
class SimpleClipSceneShader;

struct WaterSettings
{
	// water
	XMFLOAT4 colorTint         = XMFLOAT4(1, 1, 1, 1);
	float highlightStrength    = 2.0f;
	float tintFraction         = 0.2f;
	float distortionDamping    = 0.012f;
	float reflectivePower      = 0.9f;
	float fadeToDepth          = 0.5f;
	float normalScrollStrength1 = 0.6f;
	float normalScrollStrength2 = 0.6f;

	// foam
	bool applyFoam                = false;
	float foamToDepth             = 0.5f;
	float foamDistortStrength     = 2.5f;
	float foamScrollStrength      = 0.8f;
	float foamTileMultiplier      = 5.0f;
	float foamNoiseTileMultiplier = 8.0f;
	float foamDistortDamping      = 0.020f;

	XMFLOAT2 pad;
};

class WaterShader
{
public:
	WaterShader();
	~WaterShader();

	void Render(std::vector<Mesh*>& waterMeshes);
	void ShowDebugQuads();

	// the shader bytecode
	ID3D10Blob* waterVertexShaderByteCode;
	ID3D10Blob* waterPixelShaderByteCode;

private:
	
	// compiled shaders
	ID3D11VertexShader* _waterVertexShader;
	ID3D11PixelShader*  _waterPixelShader;
	
	// constant buffers
	ID3D11Buffer* _CBVertex;
	ID3D11Buffer* _CBPixel;

	SimpleClipSceneShader*  _simpleClipShaderReflection;
	SimpleClipSceneShader*  _simpleClipShaderRefraction;

	struct CBVertexWater
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
};

