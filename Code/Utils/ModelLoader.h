#pragma once
#include <vector>
#include "Mesh.h"
#include <iostream>
#include <fstream>

#include "ThirdParty/assimp/Importer.hpp"
#include "ThirdParty/assimp/scene.h"
#include "ThirdParty/assimp/postprocess.h"

class ModelLoader
{
public:
	static Mesh* CreateMesh(aiMesh* mesh, const aiScene* scene, unsigned int flags, wchar_t* diffuseMap, wchar_t* normalMap, wchar_t* specularMap, wchar_t* emissiveMap, bool useMaterial, float tiling, Entity* parent, float heightmapScale)
	{
		// structures for vertex/index data
		std::vector<Mesh::VertexData> vertices;
		std::vector<unsigned long>    indices;

		// set passed in textures, these will be used if useMaterial is set to false
		std::wstring diffuse  = diffuseMap;
		std::wstring normal   = normalMap;
		std::wstring specular = specularMap;
		std::wstring emissive = emissiveMap;

		// if we want to load the textures from a material file
		if (useMaterial)
		{
			// do this mesh have a material
			if (mesh->mMaterialIndex >= 0)
			{
				// get the material of this mesh
				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

				// diffuse map
				if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
				{
					aiString stringDiffuse;
					material->GetTexture(aiTextureType_DIFFUSE, 0, &stringDiffuse);

					diffuse = GetRelativePathAndSetExtension(stringDiffuse.C_Str(), ".dds");
				}

				// normal map (flagged as height for some reason)
				if (material->GetTextureCount(aiTextureType_HEIGHT) > 0)
				{
					aiString stringNormal;
					material->GetTexture(aiTextureType_HEIGHT, 0, &stringNormal);

					normal = GetRelativePathAndSetExtension(stringNormal.C_Str(), ".dds");
				}

				// specular map
				if (material->GetTextureCount(aiTextureType_SPECULAR) > 0)
				{
					aiString stringSpecular;
					material->GetTexture(aiTextureType_SPECULAR, 0, &stringSpecular);

					specular = GetRelativePathAndSetExtension(stringSpecular.C_Str(), ".dds");
				}

				// emissive map
				if (material->GetTextureCount(aiTextureType_EMISSIVE) > 0)
				{
					aiString emissiveSpecular;
					material->GetTexture(aiTextureType_EMISSIVE, 0, &emissiveSpecular);

					emissive = GetRelativePathAndSetExtension(emissiveSpecular.C_Str(), ".dds");
				}
			}
		}

		// loop over all vertices of this mesh
		for (UINT i = 0; i < mesh->mNumVertices; i++)
		{
			// vertex data struct
			Mesh::VertexData vertex;

			// position
			vertex.position.x = mesh->mVertices[i].x;
			vertex.position.y = mesh->mVertices[i].y;
			vertex.position.z = mesh->mVertices[i].z;

			// tex coords
			if (mesh->mTextureCoords[0])
			{
				vertex.texture.x = (float)mesh->mTextureCoords[0][i].x * tiling;
				vertex.texture.y = (float)mesh->mTextureCoords[0][i].y * tiling;
			}

			// normals
			if (mesh->HasNormals())
			{
				vertex.normal.x = mesh->mNormals[i].x;
				vertex.normal.y = mesh->mNormals[i].y;
				vertex.normal.z = mesh->mNormals[i].z;
			}

			// tangents and binormals
			if (mesh->HasTangentsAndBitangents())
			{
				vertex.tangent.x = mesh->mTangents[i].x;
				vertex.tangent.y = mesh->mTangents[i].y;
				vertex.tangent.z = mesh->mTangents[i].z;

				vertex.binormal.x = mesh->mBitangents[i].x;
				vertex.binormal.y = mesh->mBitangents[i].y;
				vertex.binormal.z = mesh->mBitangents[i].z;
			}

			// vertex colors
			if (mesh->HasVertexColors(0))
			{
				vertex.color.r = (char)mesh->mColors[0][i].r;
				vertex.color.g = (char)mesh->mColors[0][i].g;
				vertex.color.b = (char)mesh->mColors[0][i].b;
				vertex.color.a = (char)mesh->mColors[0][i].a;
			}
			else // if mesh don't have veretx colors set them to white
			{
				vertex.color.r = 255;
				vertex.color.g = 255;
				vertex.color.b = 255;
				vertex.color.a = 255;
			}

			vertices.push_back(vertex);
		}

		// loop over all faces and get the indices
		for (UINT i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];

			for (UINT j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		// create mesh and buffers
		Mesh* modelMesh = new Mesh(parent, flags, diffuse.c_str(), normal.c_str(), specular.c_str(), emissive.c_str(), HasAlpha(diffuse), NormalmapHasHeight(normal), heightmapScale);
		modelMesh->CreateBuffers(&vertices[0], &indices[0], (int)vertices.size(), (int)indices.size());

		return modelMesh;
	}

	static std::wstring GetRelativePathAndSetExtension(const char* filePath, const char* extension)
	{
		std::string filename(filePath);

		// get the offset where our last backslash is located
		// we only want the name of the texture
		size_t lastSlash = filename.find_last_of("\\");

		if (std::string::npos == lastSlash)
			lastSlash = filename.find_last_of("/");

		// erease filepath
		if (std::string::npos != lastSlash)
			filename.erase(0, lastSlash + 1);

		// change all texture exstensions to .dds so if the model
		// material was exported using other filetypes we don't have to change the .mtl file
		filename = filename.substr(0, filename.find_last_of('.')) + extension;

		// add our relative path to our textures
		std::string relativeFilePath = "Data/Textures/";
		relativeFilePath.append(filename.c_str());

		// conert to wide string (only supports asciII characters)
		std::wstring wtp(relativeFilePath.begin(), relativeFilePath.end());

		return wtp;
	}

	static bool HasAlpha(std::wstring diffuseMap)
	{
		// get the offset where our last Underscore is located
		size_t lastUnderScore = diffuseMap.find_last_of(L"_");

		// erease string to after last underscore
		if (std::string::npos != lastUnderScore) diffuseMap.erase(0, lastUnderScore + 1);
		else return false;

		// if the texture ends with _A it has alpha
		if (diffuseMap == L"A.dds")
			return true;

		return false;
	}

	static bool NormalmapHasHeight(std::wstring normalMap)
	{
		// get the offset where our last Underscore is located
		size_t lastUnderScore = normalMap.find_last_of(L"_");

		// erease string to after last underscore
		if (std::string::npos != lastUnderScore) normalMap.erase(0, lastUnderScore + 1);
		else return false;

		// if the texture ends with _H it means that the alpha channel contains the heightmap
		if (normalMap == L"H.dds")
			return true;

		return false;
	}

	static float InverseLerp(float a, float b, float t)
	{
		return (t - a) / (b - a);
	}

	// sprite that renders in 3d space without lightning
	static Mesh* CreateWorldSprite(unsigned int flags, wchar_t* diffuseMap, Entity* parent)
	{
		Mesh::VertexData* vertices = new Mesh::VertexData[4];

		vertices[0].position = XMFLOAT3(-0.5f, 0.5f, 0.0f);
		vertices[0].texture  = XMFLOAT2(0.0f, 0.0f);
		vertices[0].normal   = XMFLOAT3(0.0f, 0.0f, 0.0f);
		vertices[0].tangent  = XMFLOAT3(0.0f, 0.0f, 0.0f);
		vertices[0].binormal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		vertices[0].color    = Color32(255, 255, 255, 255);

		vertices[1].position = XMFLOAT3(0.5f, 0.5f, 0.0f);
		vertices[1].texture  = XMFLOAT2(1.0f, 0.0f);
		vertices[1].normal   = XMFLOAT3(0.0f, 0.0f, 0.0f);
		vertices[1].tangent  = XMFLOAT3(0.0f, 0.0f, 0.0f);
		vertices[1].binormal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		vertices[1].color    = Color32(255, 255, 255, 255);

		vertices[2].position = XMFLOAT3(-0.5f, -0.5f, 0.0f);
		vertices[2].texture  = XMFLOAT2(0.0f, 1.0f);
		vertices[2].normal   = XMFLOAT3(0.0f, 0.0f, 0.0f);
		vertices[2].tangent  = XMFLOAT3(0.0f, 0.0f, 0.0f);
		vertices[2].binormal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		vertices[2].color    = Color32(255, 255, 255, 255);

		vertices[3].position = XMFLOAT3(0.5f, -0.5f, 0.0f);
		vertices[3].texture  = XMFLOAT2(1.0f, 1.0f);
		vertices[3].normal   = XMFLOAT3(0.0f, 0.0f, 0.0f);
		vertices[3].tangent  = XMFLOAT3(0.0f, 0.0f, 0.0f);
		vertices[3].binormal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		vertices[3].color    = Color32(255, 255, 255, 255);

		unsigned long* indices = new unsigned long[6]{ 0,1,2,2,1,3 };

		// create mesh and buffers
		Mesh* mesh = new Mesh(parent, flags, diffuseMap, L"", L"", L"", HasAlpha(diffuseMap), false, 0);
		mesh->CreateBuffers(vertices, indices, 24, 36);

		delete[] vertices;
		delete[] indices;

		return mesh;
	}

	static Mesh* CreateSphere(unsigned int flags, wchar_t* diffuseMap, wchar_t* normalMap, wchar_t* specularMap, wchar_t* emissiveMap, float tiling, Entity* parent, float heightmapScale)
	{
		//get assimp imoprter
		Assimp::Importer importer;

		// get the scene object from the file
		const aiScene* scene = importer.ReadFile("Data/Models/sphere.obj", aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_CalcTangentSpace);

		// assert if scene failed to be created
		assert(scene != nullptr);

		return CreateMesh(scene->mMeshes[scene->mRootNode->mChildren[0]->mMeshes[0]], scene, flags, diffuseMap, normalMap, specularMap, emissiveMap, tiling, false, parent, heightmapScale);
	}
};


