#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "IComponent.h"

using namespace DirectX;

class Entity;

class QuadComponent : public IComponent
{
public:
	QuadComponent();
	~QuadComponent();

	void Init(XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale, XMFLOAT2 size, wchar_t* texturePath = L"", XMFLOAT4 color = XMFLOAT4(1, 1, 1, 1), bool ignoreAlpha = false);
	void Update(const float& delta);

	void UploadBuffers();

	ID3D11ShaderResourceView* texture;
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scale;
	XMFLOAT4 color;
	bool     ignoreAlpha;

	inline XMFLOAT4X4 GetWorldMatrix()
	{
		XMFLOAT3 rotRadian(XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z));

		XMFLOAT4X4 positionMatrix;
		XMFLOAT4X4 scaleMatrix;
		XMFLOAT4X4 rotationMatrix;

		XMFLOAT4X4 worldMatrix;

		XMStoreFloat4x4(&positionMatrix, XMMatrixTranslationFromVector(XMLoadFloat3(&position)));
		XMStoreFloat4x4(&scaleMatrix, XMMatrixScalingFromVector(XMLoadFloat3(&scale)));
		XMStoreFloat4x4(&rotationMatrix, XMMatrixRotationRollPitchYaw(rotRadian.x, rotRadian.y, rotRadian.z));

		XMStoreFloat4x4(&worldMatrix, XMMatrixMultiply(XMLoadFloat4x4(&scaleMatrix), XMLoadFloat4x4(&rotationMatrix)));
		XMStoreFloat4x4(&worldMatrix, XMMatrixMultiply(XMLoadFloat4x4(&worldMatrix), XMLoadFloat4x4(&positionMatrix)));

		return worldMatrix;
	}

private:
	
	void CreateBuffers(XMFLOAT2 size);

	// index and vertex buffer
	ID3D11Buffer* _vertexBuffer;
	ID3D11Buffer* _indexBuffer;

	// vertex inputs
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};
};

