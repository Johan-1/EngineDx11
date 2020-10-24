#pragma once
#include "IComponent.h"
#include <DirectXMath.h>
#include "TransformComponent.h"

class Entity;

class LightPointComponent : public IComponent
{
public:	
	LightPointComponent();
	~LightPointComponent();

	void Init(float radius, float intensity, XMFLOAT3 color, float attConstant = 0.0f, float attLinear = 1.0f, float attExponential = 0.0f);

	void Update(const float& delta);

	// light properties
	XMFLOAT3 color;
	float intensity;
	float radius;
	float attConstant;
	float attLinear;
	float attExponential;
};

