#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "TransformComponent.h"
#include "Color32.h"
#include "WaterShader.h"

#define STANDARD                  1 << 0
#define CAST_SHADOW_DIR           1 << 1
#define CAST_REFLECTION           1 << 2
#define WIREFRAME_COLOR           1 << 3
#define PLANAR_REFLECTION         1 << 4
#define WATER                     1 << 5
#define REFRACT                   1 << 6
#define INSTANCED                 1 << 7
#define INSTANCED_CAST_SHADOW_DIR 1 << 8
#define INSTANCED_CAST_REFLECTION 1 << 9
#define INSTANCED_REFRACT         1 << 10

using namespace DirectX;

class Mesh
{
public:	
	Mesh(Entity* parent, unsigned int FLAGS, const wchar_t* diffuseMap, const wchar_t* normalMap, const wchar_t* metalicMap, const wchar_t* rougnessMap, const wchar_t* emissiveMap, bool hasAlpha, bool hasHeightmap, float heightMapScale);
	~Mesh();

	// vertex data structure
	struct VertexData
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
		XMFLOAT3 tangent;
		XMFLOAT3 binormal;
		Color32  color;
	};

	// create vertex and index buffers
	void CreateBuffers(VertexData* verticesData, unsigned long* indicesData, unsigned int numVertices, unsigned int numIndices);

	// uploads the buffers before rendering
	void UploadBuffers();

	// get the world matrix of the transform this mesh belongs to
	// get the position of the transform this mesh belongs to
	const XMFLOAT4X4& GetWorldMatrix()      { return _transform->worldMatrix; }
	const XMFLOAT4X4& GetWorldMatrixTrans() { return _transform->worldMatrixTrans; }
	const XMFLOAT3& GetPosition()           { return _transform->position; }	

	// adds and removes this mesh to/from the renderer
	void AddRemoveToRenderer(bool add);
	
	// buffers
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;	

	// num vertices/indices
	unsigned int numVertices;
	unsigned int numIndices;

	// textures
	ID3D11ShaderResourceView* baseTextures[5];
	ID3D11ShaderResourceView* DUDVMap;
	ID3D11ShaderResourceView* foamMap;
	ID3D11ShaderResourceView* noiseMap;
	ID3D11ShaderResourceView* normalMap2;

	unsigned int FLAGS;
	bool         hasHeightmap;	
	bool         hasAlpha;
	float        camDistance;
	float        heightMapScale;	
	float        planarReflectionFraction;
	XMFLOAT2     uvOffset;

	WaterSettings waterSettings;

	float metalic;
	float rougness;

private:	

	// pointer to transform component
	TransformComponent* _transform;
};

