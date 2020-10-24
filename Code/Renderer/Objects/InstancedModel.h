#pragma once
#include "ModelComponent.h"
#include <vector>
#include <DirectXMath.h>

#include "ThirdParty/assimp/Importer.hpp"
#include "ThirdParty/assimp/scene.h"
#include "ThirdParty/assimp/postprocess.h"

class mesh;

using namespace DirectX;

struct ModelInstance
{
	XMFLOAT4X4 worldMatrix;

	ModelInstance(XMFLOAT4X4 matrix) :
		worldMatrix(matrix) {}
};

class InstancedModel
{
public:
	InstancedModel(char* model, unsigned int flags, wchar_t* diffuseMap = L"", wchar_t* normalMap = L"", wchar_t* specularMap = L"", wchar_t* emissiveMap = L"", bool useMaterial = true, float tiling = 1.0f, float heightMapScale = 0.04f);
	~InstancedModel();

	void AddInstance(XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale);

	// build the instance buffer from a list of world matrices
	// can either send in a custom list of instances or use the member list
	// of this class
	void BuildInstanceBuffer(std::vector<ModelInstance>& instances);
	void BuildInstanceBuffer();

	// render all instances of this model
	void UploadInstances();

	// meshes data
	std::vector<Mesh*> meshes;
	unsigned int       numMeshes;
	unsigned int	   numInstances;
	unsigned int       strides[2];
	unsigned int       offsets[2];

	std::vector<ModelInstance> modelInstances;

private:

	void BuildBuffer(std::vector<ModelInstance>& instances);

	void ProcessNode(aiNode* node, const aiScene* scene, wchar_t* diffuseMap, wchar_t* normalMap, wchar_t* specularMap, wchar_t* emissiveMap, bool useMaterial, float tiling, float heightMapScale);
	void AddToRenderQueues(bool add);

	ID3D11Buffer* _instanceBuffer;

	// render flags and if model have a .mtl file
	unsigned int _FLAGS;
	bool         _useMaterial;
};

