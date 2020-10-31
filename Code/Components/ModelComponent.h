#pragma once
#include <d3d11.h>
#include "IComponent.h"
#include <vector>
#include "Color32.h"
#include "Mesh.h"
#include "Systems.h"
#include "TexturePool.h"

#include "ThirdParty/assimp/Importer.hpp"
#include "ThirdParty/assimp/scene.h"
#include "ThirdParty/assimp/postprocess.h"

class Entity;
class Mesh;

class ModelComponent : public IComponent
{
public:
	ModelComponent();
	~ModelComponent();

	// creates a model from file
	void InitModel(char* model, unsigned int flags, wchar_t* diffuseMap = L"", wchar_t* normalMap = L"", wchar_t* metalicMap = L"", wchar_t* rougnessMap = L"", wchar_t* emissiveMap = L"", bool useMaterial = true, float tiling = 1.0f, float heightMapScale = 0.04f);
	void Update(const float& delta);

	// overides and calls base
	// also removes/adds all meshes to renderer
	void SetActive(bool) override;

	// set render flags on all meshes in this model
	void SetRenderFlags(unsigned int flags); 

	// adds a texturemap to all meshes in model
	void SetDUDVMap(const wchar_t* texture)    { for (int i = 0; i < meshes.size(); i++) meshes[i]->DUDVMap    = Systems::texturePool->GetTexture(texture); }
	void SetFoamMap(const wchar_t* texture)    { for (int i = 0; i < meshes.size(); i++) meshes[i]->foamMap    = Systems::texturePool->GetTexture(texture); }
	void SetNoiseMap(const wchar_t* texture)   { for (int i = 0; i < meshes.size(); i++) meshes[i]->noiseMap   = Systems::texturePool->GetTexture(texture); }
	void SetNormalMap2(const wchar_t* texture) { for (int i = 0; i < meshes.size(); i++) meshes[i]->normalMap2 = Systems::texturePool->GetTexture(texture); }

	std::vector<Mesh*> meshes;
	unsigned int       numMeshes;

private:

	void ProcessNode(aiNode* node, const aiScene* scene, wchar_t* diffuseMap, wchar_t* normalMap, wchar_t* metalicMap, wchar_t* rougnessMap, wchar_t* emissiveMap, bool useMaterial, float tiling, float heightMapScale);

	bool         _useMaterial;
	unsigned int _FLAGS;
};

