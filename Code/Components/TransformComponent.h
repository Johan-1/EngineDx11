#pragma once
#include "IComponent.h"

#include <DirectXMath.h>

using namespace DirectX;

class TransformComponent : public IComponent
{
public:
	TransformComponent();
	~TransformComponent();

	void Init(XMFLOAT3 position = XMFLOAT3(0, 0, 0), XMFLOAT3 rotation = XMFLOAT3(0, 0, 0), XMFLOAT3 scale = XMFLOAT3(1, 1, 1));
	void Update(const float& delta);

	// get individual directions from transform
	XMFLOAT3  GetRight();
	XMFLOAT3  GetForward();
	XMFLOAT3  GetUp();

	// get all directions of transform
	void GetAllAxis(XMFLOAT3& forward, XMFLOAT3& right, XMFLOAT3& up);

	// add amount to position, rotation and scale
	void AddTranslation(XMFLOAT3& amount);
	void AddRotation(XMFLOAT3& amount);
	void AddScale(XMFLOAT3& amount);

	// updates the entire world matrix from all other matrices
	void BuildWorldMatrix();

	// vector3 representations
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 scale;

	// matrix representations
	XMFLOAT4X4 positionMatrix;
	XMFLOAT4X4 rotationMatrix;
	XMFLOAT4X4 scaleMatrix;
	XMFLOAT4X4 worldMatrix;
	XMFLOAT4X4 worldMatrixTrans;
};

