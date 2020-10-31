#include "ModelComponent.h"
#include <DirectXMath.h>
#include "Entity.h"
#include "ModelLoader.h"

using namespace DirectX;

ModelComponent::ModelComponent() : IComponent(COMPONENT_TYPE::MODEL_COMPONENT)
{
}

ModelComponent::~ModelComponent()
{
	for (int i = 0; i < numMeshes; i++)
		delete meshes[i];
}

void ModelComponent::InitModel(char* model, unsigned int flags, wchar_t* diffuseMap, wchar_t* normalMap, wchar_t* metalicMap, wchar_t* rougnessMap, wchar_t* emissiveMap, bool useMaterial, float tiling, float heightMapScale)
{
	_FLAGS = flags;
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
}

void ModelComponent::Update(const float& delta)
{	
}

void ModelComponent::SetActive(bool active) 
{
	// return if the state we are trying to set is the 
	// same that we already is in
	if (active == _isActive)
		return;

	// set active flag in component base
	IComponent::SetActive(active);
	
	// remove/add meshes to renderer
	for (int i = 0; i < meshes.size(); i++)
		meshes[i]->AddRemoveToRenderer(active);
}

// set the flag on all meshes in this model
void ModelComponent::SetRenderFlags(unsigned int flags)
{ 
	// do nothing if the component is inactive or if we are trying to set the same flags we already have
	if (!_isActive || (_FLAGS & flags) == _FLAGS)
		return;

	// set the new flags
	_FLAGS = flags;

	// remove our meshes from the renderer
	// set new render flags and re add them to the renderer
	for (int i = 0; i < numMeshes; i++)
	{
		meshes[i]->AddRemoveToRenderer(false);
		meshes[i]->FLAGS = flags;
		meshes[i]->AddRemoveToRenderer(true);
	}
}

void ModelComponent::ProcessNode(aiNode* node, const aiScene* scene, wchar_t* diffuseMap, wchar_t* normalMap, wchar_t* metalicMap, wchar_t* rougnessMap, wchar_t* emissiveMap, bool useMaterial, float tiling, float heightMapScale)
{
	// get and create all meshes in this node
	for (UINT i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(ModelLoader::CreateMesh(mesh, scene, _FLAGS, diffuseMap, normalMap, metalicMap, rougnessMap, emissiveMap, _useMaterial, tiling, parent, heightMapScale));
	}

	// recursivly loop over and process all child nodes
	for (UINT i = 0; i < node->mNumChildren; i++)	
		ProcessNode(node->mChildren[i], scene, diffuseMap, normalMap, metalicMap, rougnessMap, emissiveMap, useMaterial, tiling, heightMapScale);
}


