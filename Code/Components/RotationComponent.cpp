#include "RotationComponent.h"
#include "Time.h"
#include "Systems.h"

RotationComponent::RotationComponent() : IComponent(COMPONENT_TYPE::TRANSFORMATION_COMPONENT)
{
}

RotationComponent::~RotationComponent()
{
}

void RotationComponent::Init(XMFLOAT3 rotation, float speedRotation)
{
	_rotation      = rotation;
	_speedRotation = speedRotation;
	
	_transform = GetComponent<TransformComponent>();
}

void RotationComponent::Update(const float& delta)
{	
	// get rotation amount
	float rd = _speedRotation * delta;

	// rotation to add
	XMFLOAT3 newRotation(0, 0, 0);
	XMFLOAT3 rotationAmount(rd, rd, rd);

	// get amount to add to rotation
	XMStoreFloat3(&newRotation, 
		XMVectorMultiply(XMLoadFloat3(&_rotation), XMLoadFloat3(&rotationAmount)));

	// add rotation and build world matrix
	_transform->AddRotation(newRotation);
	_transform->BuildWorldMatrix();
	
}