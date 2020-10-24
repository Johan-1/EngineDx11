#include "TransformComponent.h"

TransformComponent::TransformComponent() : IComponent(COMPONENT_TYPE::TRANSFORM_COMPONENT)
{	
}

TransformComponent::~TransformComponent()
{	
}

// inits the transform properties and builds the world matrix
void TransformComponent::Init(XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale)
{
	this->position = position;
	this->rotation = rotation;
	this->scale    = scale;

	BuildWorldMatrix();
}

// add value to position
void TransformComponent::AddTranslation(XMFLOAT3& amount)
{
	XMStoreFloat3(&position, XMVectorAdd(XMLoadFloat3(&amount), XMLoadFloat3(&position)));
}

// add value to rotation
void TransformComponent::AddRotation(XMFLOAT3& amount)
{
	XMStoreFloat3(&rotation, XMVectorAdd(XMLoadFloat3(&amount), XMLoadFloat3(&rotation)));
}

// add value to scale
void TransformComponent::AddScale(XMFLOAT3& amount)
{
	XMStoreFloat3(&scale, XMVectorAdd(XMLoadFloat3(&amount), XMLoadFloat3(&scale)));
}

// calculate all matrices for position, scale, rotation and mutliply them all together
void TransformComponent::BuildWorldMatrix() 
{	
	XMFLOAT3 rotRadian(XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z));

	XMStoreFloat4x4(&positionMatrix, XMMatrixTranslationFromVector(XMLoadFloat3(&position)));
	XMStoreFloat4x4(&scaleMatrix,    XMMatrixScalingFromVector(XMLoadFloat3(&scale)));
	XMStoreFloat4x4(&rotationMatrix, XMMatrixRotationRollPitchYaw(rotRadian.x, rotRadian.y, rotRadian.z));

	XMStoreFloat4x4(&worldMatrix, XMMatrixMultiply(XMLoadFloat4x4(&scaleMatrix), XMLoadFloat4x4(&rotationMatrix)));
	XMStoreFloat4x4(&worldMatrix, XMMatrixMultiply(XMLoadFloat4x4(&worldMatrix), XMLoadFloat4x4(&positionMatrix)));

	XMStoreFloat4x4(&worldMatrixTrans, XMMatrixTranspose(XMLoadFloat4x4(&worldMatrix)));
}

// get our forward orientation
XMFLOAT3 TransformComponent::GetForward()
{
	XMFLOAT3 f = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMStoreFloat3(&f, XMVector3Normalize(XMVector3TransformCoord(XMLoadFloat3(&f), XMLoadFloat4x4(&rotationMatrix))));

	return f;
}

// get our right orientation
XMFLOAT3 TransformComponent::GetRight()
{
	XMFLOAT3 r = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMStoreFloat3(&r, XMVector3Normalize(XMVector3TransformCoord(XMLoadFloat3(&r), XMLoadFloat4x4(&rotationMatrix))));

	return r;
}

// get our up orientation
XMFLOAT3 TransformComponent::GetUp()
{
	XMFLOAT3 u = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMStoreFloat3(&u, XMVector3Normalize(XMVector3TransformCoord(XMLoadFloat3(&u), XMLoadFloat4x4(&rotationMatrix))));

	return u;
}

// set all orientations by the references passed in
void TransformComponent::GetAllAxis(XMFLOAT3& forward, XMFLOAT3& right, XMFLOAT3& up)
{
	XMFLOAT3 f = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMFLOAT3 r = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3 u = XMFLOAT3(0.0f, 1.0f, 0.0f);

	XMStoreFloat3(&forward, XMVector3Normalize(XMVector3TransformCoord(XMLoadFloat3(&f), XMLoadFloat4x4(&rotationMatrix))));
	XMStoreFloat3(&right,   XMVector3Normalize(XMVector3TransformCoord(XMLoadFloat3(&r), XMLoadFloat4x4(&rotationMatrix))));
	XMStoreFloat3(&up,      XMVector3Normalize(XMVector3TransformCoord(XMLoadFloat3(&u), XMLoadFloat4x4(&rotationMatrix))));
}

void TransformComponent::Update(const float& delta){}

