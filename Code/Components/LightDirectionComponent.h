#pragma once
#include <DirectXMath.h>
#include "IComponent.h"
#include "TransformComponent.h"

using namespace DirectX;

class Entity;

class LightDirectionComponent : public IComponent
{
public:
	LightDirectionComponent();
	~LightDirectionComponent();

	void Init(XMFLOAT4 lightColor, TransformComponent* transformOverride = nullptr);
	
	void Update(const float& delta);

	// get the look direction of this light
	XMFLOAT3 GetLightDirection();
	XMFLOAT3 GetLightDirectionInv(); 

	// light color
	XMFLOAT4 lightColor;

private:

	// pointer to transform
	TransformComponent* _transform;

	// pointer to transform to overide light direction
	TransformComponent* _overrideTransform;

	
};

