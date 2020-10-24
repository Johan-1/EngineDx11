#include "LightPointComponent.h"
#include "Entity.h"
#include "LightManager.h"
#include "Systems.h"

LightPointComponent::LightPointComponent() : IComponent(POINT_LIGHT_COMPONENT)
{
}

LightPointComponent::~LightPointComponent()
{
}

// set light properties
void LightPointComponent::Init(float radius, float intensity, XMFLOAT3 color, float attConstant, float attLinear, float attExponential)
{
	this->radius         = radius;
	this->intensity      = intensity;
	this->color          = color;
	this->attConstant    = attConstant;
	this->attLinear      = attLinear;
	this->attExponential = attExponential;

	Systems::lightManager->AddPointLight(this);
}

void LightPointComponent::Update(const float& delta)
{
}