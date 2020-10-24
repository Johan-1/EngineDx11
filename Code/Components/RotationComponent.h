#pragma once
#include "IComponent.h"
#include <DirectXMath.h>
#include "TransformComponent.h"

using namespace DirectX;

class Entity;

class RotationComponent : public IComponent
{
public:	
	RotationComponent();
	~RotationComponent();

	void Init(XMFLOAT3 rotation, float speedRotation);
	void Update(const float& delta);

private:

	// pointer to transform component
	TransformComponent* _transform;

	// rotation
	XMFLOAT3 _rotation;

	// rotation speed
	float _speedRotation;	
};

