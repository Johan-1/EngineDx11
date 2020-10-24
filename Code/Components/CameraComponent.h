#pragma once
#include <D3D11.h>
#include <DirectXMath.h>
#include "IComponent.h"

using namespace DirectX;

class TransformComponent;

class CameraComponent : public IComponent
{
public:
	CameraComponent();
	~CameraComponent();

	// init for perspective and orthographic
	void Init3D(const float& fov);
	void Init2D(const XMFLOAT2& size, const XMFLOAT2& nearfar);
	
	// update from pure virtual in IComponent
	void Update(const float& delta);
	
	// calculates the camera viewmatrix
	void CalculateViewMatrix();

	// camera matrices
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projectionMatrix;
	XMFLOAT4X4 viewProjMatrix;
	XMFLOAT4X4 viewProjMatrixTrans;

	// pointer to a render texture
	ID3D11ShaderResourceView* renderTexture;

private:

	// cached pointer to transform component
	TransformComponent* _transform;
};

