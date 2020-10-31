#include "Mesh.h"
#include "DXManager.h"
#include "Renderer.h"
#include "TexturePool.h"
#include "Systems.h"
#include "DXErrorhandler.h"

Mesh::Mesh(Entity* parent, unsigned int FLAGS, const wchar_t* diffuseMap, const wchar_t* normalMap, const wchar_t* metalicMap, const wchar_t* rougnessMap, const wchar_t* emissiveMap, bool hasAlpha, bool hasHeightmap, float heightMapScale) :
	uvOffset(XMFLOAT2(0,0)),
	FLAGS(FLAGS),
	hasAlpha(hasAlpha),
	hasHeightmap(hasHeightmap),
	heightMapScale(heightMapScale)
{
	metalic  = 1.0f;
	rougness = 1.0f;

	// get pointer to transform component
	if (parent != nullptr)
		_transform = parent->GetComponent<TransformComponent>();

	// load into wstrings so we can check if they are empty
	std::wstring diffuse(diffuseMap);
	std::wstring normal(normalMap);
	std::wstring metalic(metalicMap);
	std::wstring rougness(rougnessMap);
	std::wstring emissive(emissiveMap);

	// set passed in textures or defualt ones
	TexturePool& TP                     = *Systems::texturePool;
	!diffuse.empty()  ? baseTextures[0] = TP.GetTexture(diffuseMap)  : baseTextures[0] = TP.GetTexture(L"Data/Textures/defaultDiffuse.dds");
	!normal.empty()   ? baseTextures[1] = TP.GetTexture(normalMap)   : baseTextures[1] = TP.GetTexture(L"Data/Textures/defaultNormal.dds");
	!metalic.empty()  ? baseTextures[2] = TP.GetTexture(metalicMap)  : baseTextures[2] = TP.GetTexture(L"Data/Textures/white.dds");
	!rougness.empty() ? baseTextures[3] = TP.GetTexture(rougnessMap) : baseTextures[3] = TP.GetTexture(L"Data/Textures/white.dds");
	!emissive.empty() ? baseTextures[4] = TP.GetTexture(emissiveMap) : baseTextures[4] = TP.GetTexture(L"Data/Textures/defualtEmissive.dds");

	// add this mesh to the renderer
	AddRemoveToRenderer(true);
}

Mesh::~Mesh()
{	
	// release buffers
	vertexBuffer->Release();
	indexBuffer->Release();

	// remove this mesh from renderer
	AddRemoveToRenderer(false);
}

void Mesh::CreateBuffers(VertexData* verticesData, unsigned long* indicesData, unsigned int numVertices, unsigned int numIndices) 
{
	// save number of vertices/indices
	this->numVertices = numVertices;
	this->numIndices  = numIndices;

	ID3D11Device*& device = Systems::dxManager->device;

	// create the descriptions and rasourcedata to buffers
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Set up the description of the vertex buffer.
	vertexBufferDesc.Usage               = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.ByteWidth           = sizeof(VertexData) * numVertices;
	vertexBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags      = 0;
	vertexBufferDesc.MiscFlags           = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Set up the description of the index buffer.
	indexBufferDesc.Usage               = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth           = sizeof(unsigned long) * numIndices;
	indexBufferDesc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags      = 0;
	indexBufferDesc.MiscFlags           = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem          = verticesData;
	vertexData.SysMemPitch      = 0;
	vertexData.SysMemSlicePitch = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem          = indicesData;
	indexData.SysMemPitch      = 0;
	indexData.SysMemSlicePitch = 0;

	HRESULT result;

	// create vertex and index buffers
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create vertex buffer for mesh");

	result = device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create index buffer for mesh");
}

void Mesh::UploadBuffers() 
{
	ID3D11DeviceContext*& devCon = Systems::dxManager->devCon;

	// set stride and offset of one vertex
	unsigned int stride = sizeof(VertexData);
	unsigned int offset = 0;
	
	// Set the vertex buffer 
	devCon->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	// Set the index buffer 
	devCon->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
}

void Mesh::AddRemoveToRenderer(bool add)
{
	Renderer& renderer = *Systems::renderer;

	// will render with the regular Deferred render pass for opaque objects or with forward rendering for alpha objects
	// the standard shader renders directional and point lights, directional shadows, supports normal,specular and emissive maps
	if ((FLAGS & STANDARD) == STANDARD)
	{
		add ? renderer.AddMeshToRenderer(this, SHADER_TYPE::S_DEFERRED) : renderer.RemoveMeshFromRenderer(this, SHADER_TYPE::S_DEFERRED);
	}

	// if the meshes should be included when rendering reflection maps
	if ((FLAGS & CAST_REFLECTION) == CAST_REFLECTION)
	{
		add ? renderer.AddMeshToRenderer(this, SHADER_TYPE::S_CAST_REFLECTION) : renderer.RemoveMeshFromRenderer(this, SHADER_TYPE::S_CAST_REFLECTION);
	}

	// if the mesh should be rendered to refraction maps
	if ((FLAGS & REFRACT) == REFRACT)
	{
		add ? renderer.AddMeshToRenderer(this, SHADER_TYPE::S_REFRACT_OPAQUE) : renderer.RemoveMeshFromRenderer(this, SHADER_TYPE::S_REFRACT_OPAQUE);
	}

	// add to depth rendering for shadow casting
	// at the moment all meshes recive shadows
	if ((FLAGS & CAST_SHADOW_DIR) == CAST_SHADOW_DIR)
		add ? renderer.AddMeshToRenderer(this, SHADER_TYPE::S_DEPTH) : renderer.RemoveMeshFromRenderer(this, SHADER_TYPE::S_DEPTH);
		
	// forward rendered alpha meshes with planar reflections
	if ((FLAGS & PLANAR_REFLECTION) == PLANAR_REFLECTION)
		add ? renderer.AddMeshToRenderer(this, SHADER_TYPE::S_PLANAR_REFLECTION) : renderer.RemoveMeshFromRenderer(this, SHADER_TYPE::S_PLANAR_REFLECTION);

	// forward rendered water meshes, renders both a reflection and refractionmap
	// a DUDV map can be set by calling the model
	if ((FLAGS & WATER) == WATER)
		add ? renderer.AddMeshToRenderer(this, SHADER_TYPE::S_WATER) : renderer.RemoveMeshFromRenderer(this, SHADER_TYPE::S_WATER);

	// debug meshes that only renders a colored wireframe
	if ((FLAGS & WIREFRAME_COLOR) == WIREFRAME_COLOR)
		add ? renderer.AddMeshToRenderer(this, SHADER_TYPE::S_WIREFRAME) : renderer.RemoveMeshFromRenderer(this, SHADER_TYPE::S_WIREFRAME);
}

