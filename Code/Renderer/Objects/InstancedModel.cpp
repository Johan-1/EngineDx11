#include "InstancedModel.h"
#include "ModelLoader.h"
#include "DXManager.h"
#include "DXErrorHandler.h"
#include "Systems.h"
#include "Mesh.h"
#include "MathHelpers.h"

InstancedModel::InstancedModel(char* model, unsigned int flags, wchar_t* diffuseMap, wchar_t* normalMap, wchar_t* metalicMap, wchar_t* rougnessMap, wchar_t* emissiveMap, bool useMaterial, float tiling, float heightMapScale)
{
	_FLAGS       = flags;
	_useMaterial = useMaterial;

	//get assimp imoprter
	Assimp::Importer importer;

	// get the scene object from the file
	const aiScene* scene = importer.ReadFile(model, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);

	// assert if scene failed to be created
	assert(scene != nullptr);

	// send the root node and recurivly create all meshes
	ProcessNode(scene->mRootNode, scene, diffuseMap, normalMap, metalicMap, rougnessMap, emissiveMap, useMaterial, tiling, heightMapScale);

	// get how many meshes that was loaded
	numMeshes = meshes.size();

	AddToRenderQueues(true);

	// Set vertex buffers stride and offset.
	strides[0] = sizeof(Mesh::VertexData);
	strides[1] = sizeof(ModelInstance);

	offsets[0] = 0;
	offsets[1] = 0;
}

InstancedModel::~InstancedModel()
{
	_instanceBuffer->Release();
}

void InstancedModel::AddInstance(XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale)
{
	modelInstances.emplace_back((ModelInstance(MATH_HELPERS::CreateWorldMatrix(position, rotation, scale))));
}

void InstancedModel::BuildInstanceBuffer()
{
	BuildBuffer(modelInstances);
}

void InstancedModel::BuildInstanceBuffer(std::vector<ModelInstance>& instances)
{
	BuildBuffer(instances);
}

void InstancedModel::BuildBuffer(std::vector<ModelInstance>& instances)
{
	D3D11_MAPPED_SUBRESOURCE data;
	ID3D11DeviceContext*& devCon = Systems::dxManager->devCon;
	numInstances                = instances.size();

	if (_instanceBuffer == nullptr)
	{
		// create the descriptions and resource data to buffers
		D3D11_BUFFER_DESC instanceBufferDesc;

		// Set up the description of the instance buffer.
		instanceBufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
		instanceBufferDesc.ByteWidth           = sizeof(ModelInstance) * numInstances;
		instanceBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
		instanceBufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
		instanceBufferDesc.MiscFlags           = 0;
		instanceBufferDesc.StructureByteStride = 0;

		HRESULT result = Systems::dxManager->device->CreateBuffer(&instanceBufferDesc, NULL, &_instanceBuffer);
		if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create instance buffer for instanced model");
	}
	else
	{
		D3D11_BUFFER_DESC old;
		_instanceBuffer->GetDesc(&old);

		if (old.ByteWidth < sizeof(ModelInstance) * numInstances)
		{
			// realease old buffer, is now to small
			_instanceBuffer->Release();

			// create new buffer with increased size so we have room to expand
			D3D11_BUFFER_DESC instanceBufferDesc;
			instanceBufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
			instanceBufferDesc.ByteWidth           = (sizeof(ModelInstance) * numInstances) * 1.5f;
			instanceBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
			instanceBufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
			instanceBufferDesc.MiscFlags           = 0;
			instanceBufferDesc.StructureByteStride = 0;

			HRESULT result = Systems::dxManager->device->CreateBuffer(&instanceBufferDesc, NULL, &_instanceBuffer);
			if (FAILED(result)) DX_ERROR::PrintError(result, "failed to create instance buffer for instanced model");
		}		
	}
	
	// map instancebuffer
	HRESULT result = devCon->Map(_instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &data);
	if (FAILED(result)) DX_ERROR::PrintError(result, "failed to map instance buffer for instanced model");

	// copy the instancedata over to the instancebuffer
	memcpy(data.pData, (void*)&instances[0], sizeof(ModelInstance) * instances.size());

	//unmap
	devCon->Unmap(_instanceBuffer, 0);
}

void InstancedModel::UploadInstances()
{
	// set the instance buffer in slot 1
	Systems::dxManager->devCon->IASetVertexBuffers(1, 1, &_instanceBuffer, &strides[1], &offsets[1]);
}

void InstancedModel::ProcessNode(aiNode* node, const aiScene* scene, wchar_t* diffuseMap, wchar_t* normalMap, wchar_t* metalicMap, wchar_t* rougnessMap, wchar_t* emissiveMap, bool useMaterial, float tiling, float heightMapScale)
{
	// get and create all meshes in this node
	for (UINT i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(ModelLoader::CreateMesh(mesh, scene, 0, diffuseMap, normalMap, metalicMap, rougnessMap, emissiveMap, _useMaterial, tiling, nullptr, heightMapScale));
	}

	// recursivly loop over and process all child nodes
	for (UINT i = 0; i < node->mNumChildren; i++)
		ProcessNode(node->mChildren[i], scene, diffuseMap, normalMap, metalicMap, rougnessMap, emissiveMap, useMaterial, tiling, heightMapScale);
}

void InstancedModel::AddToRenderQueues(bool add)
{
	Renderer& renderer = *Systems::renderer;

	if ((_FLAGS & INSTANCED) == INSTANCED)
		add ? renderer.AddInstancedModelToRenderer(this, INSTANCED_SHADER_TYPE::S_INSTANCED_DEFERRED) : renderer.RemoveInstancedModelFromRenderer(this, INSTANCED_SHADER_TYPE::S_INSTANCED_DEFERRED);

	if ((_FLAGS & INSTANCED_CAST_SHADOW_DIR) == INSTANCED_CAST_SHADOW_DIR)
		add ? renderer.AddInstancedModelToRenderer(this, INSTANCED_SHADER_TYPE::S_INSTANCED_DEPTH) : renderer.RemoveInstancedModelFromRenderer(this, INSTANCED_SHADER_TYPE::S_INSTANCED_DEPTH);

	if ((_FLAGS & INSTANCED_CAST_REFLECTION) == INSTANCED_CAST_REFLECTION)
		add ? renderer.AddInstancedModelToRenderer(this, INSTANCED_SHADER_TYPE::S_INSTANCED_CAST_REFLECTION) : renderer.RemoveInstancedModelFromRenderer(this, INSTANCED_SHADER_TYPE::S_INSTANCED_CAST_REFLECTION);

	if ((_FLAGS & INSTANCED_REFRACT) == INSTANCED_REFRACT)
		add ? renderer.AddInstancedModelToRenderer(this, INSTANCED_SHADER_TYPE::S_INSTANCED_REFRACT) : renderer.RemoveInstancedModelFromRenderer(this, INSTANCED_SHADER_TYPE::S_INSTANCED_REFRACT);
}