#pragma once
#include <d3d11.h>
#include <vector>
#include <DirectXMath.h>

class Mesh;
class GBuffer;
class InstancedModel;

using namespace DirectX;

class DeferredShader
{
public:
	DeferredShader();
	~DeferredShader();

	// renders all meshes to the depth map
	void RenderGeometry(std::vector<Mesh*>& meshes);
	void renderGeometryInstanced(std::vector<InstancedModel*> models);
	void RenderLightning(GBuffer*& gBuffer);

	// the shader bytecode
	ID3D10Blob* vertexGeometryShaderByteCode;
	ID3D10Blob* vertexGeometryShaderByteCodeInstanced;
	ID3D10Blob* pixelGeometryShaderByteCode;
	ID3D10Blob* vertexLightningShaderByteCode;
	ID3D10Blob* pixelLightningShaderByteCode;

private:

	// compiled shaders
	ID3D11VertexShader* _vertexGeometryShader;
	ID3D11VertexShader* _vertexGeometryShaderInstanced;
	ID3D11PixelShader*  _pixelGeometryShader;

	ID3D11VertexShader* _vertexLightShader;
	ID3D11PixelShader*  _pixelLightShader;

	// constant buffers
	ID3D11Buffer* _CBGeometryVertex;
	ID3D11Buffer* _CBGeometryPixel;
	ID3D11Buffer* _CBGeometryVertexInstanced;
	ID3D11Buffer* _CBMisc;

	// constant data structure for the geometry pass
	struct CBGeometryVertex
	{
		XMFLOAT4X4 world;
		XMFLOAT4X4 worldViewProj;
		XMFLOAT2   UVOffset;
		XMFLOAT2   pad;
	};

	struct CBGeometryPixel
	{
		XMFLOAT4 cameraPos;
		int      hasHeightmap;
		float    heightScale;
		XMFLOAT2 pad;
	};

	struct CBGeometryVertexInstanced
	{
		XMFLOAT4X4 ViewProj;	
	};

	struct CBMiscPixel
	{
		XMFLOAT4X4 lightViewProj;
		XMFLOAT4   cameraPosition;
	};

};

